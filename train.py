# train.py
import os
import time
import pickle
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torchvision import datasets, transforms
from torch.utils.data import ConcatDataset
from tqdm import tqdm
import matplotlib.pyplot as plt

# ======================= 配置参数 =======================
class Args:
    data = "./data"               # 数据集根目录
    batch_size = 64
    epochs = 15                   # 最大训练轮数（早停会提前结束）
    lr = 0.001                    # 初始学习率
    seed = 1
    use_gpu = torch.cuda.is_available()
    results_dir = "./results"     # 模型保存目录
    # 早停参数
    patience = 5                  # 验证准确率连续多少个epoch不提升就停止
    min_delta = 0.001             # 最小改善阈值（0.1%）

args = Args()
torch.manual_seed(args.seed)
if args.use_gpu:
    torch.cuda.manual_seed(args.seed)
    print("[INFO] Using GPU")
else:
    print("[INFO] Using CPU")

os.makedirs(args.results_dir, exist_ok=True)

# ======================= 数据处理 =======================
def char_order2int_order(charoder):
    try:
        return int(charoder)
    except:
        print(f"Warning: invalid class folder name: {charoder}")
        return -1

data_transforms = transforms.Compose([
    transforms.Resize((32, 32)),
    transforms.ToTensor(),
    transforms.Normalize((0.3337, 0.3064, 0.3171), (0.2672, 0.2564, 0.2629))
])

data_jitter_brightness = transforms.Compose([
    transforms.Resize((32, 32)),
    transforms.ColorJitter(brightness=0.5),
    transforms.ToTensor(),
    transforms.Normalize((0.3337, 0.3064, 0.3171), (0.2672, 0.2564, 0.2629))
])

def processing_data(data_path, batch_size, use_gpu):
    train_dataset = datasets.ImageFolder(
        data_path + '/train_images',
        transform=data_transforms,
        target_transform=char_order2int_order
    )
    train_dataset_aug = datasets.ImageFolder(
        data_path + '/train_images',
        transform=data_jitter_brightness,
        target_transform=char_order2int_order
    )
    full_train_dataset = ConcatDataset([train_dataset, train_dataset_aug])
    val_dataset = datasets.ImageFolder(
        data_path + '/val_images',
        transform=data_transforms,
        target_transform=char_order2int_order
    )
    train_loader = torch.utils.data.DataLoader(
        full_train_dataset, batch_size=batch_size, shuffle=True,
        num_workers=0, pin_memory=use_gpu
    )
    val_loader = torch.utils.data.DataLoader(
        val_dataset, batch_size=batch_size, shuffle=False,
        num_workers=0, pin_memory=use_gpu
    )
    return train_loader, val_loader

# ======================= 改进的CNN模型 =======================
class ImprovedNet(nn.Module):
    def __init__(self, nclasses=43):
        super(ImprovedNet, self).__init__()
        self.conv1 = nn.Conv2d(3, 64, kernel_size=3, padding=1)
        self.bn1 = nn.BatchNorm2d(64)
        self.conv2 = nn.Conv2d(64, 128, kernel_size=3, padding=1)
        self.bn2 = nn.BatchNorm2d(128)
        self.conv3 = nn.Conv2d(128, 256, kernel_size=3, padding=1)
        self.bn3 = nn.BatchNorm2d(256)
        self.conv4 = nn.Conv2d(256, 512, kernel_size=3, padding=1)
        self.bn4 = nn.BatchNorm2d(512)
        self.pool = nn.MaxPool2d(2, 2)
        self.dropout = nn.Dropout(0.5)
        self.fc1 = nn.Linear(512 * 2 * 2, 1024)
        self.fc2 = nn.Linear(1024, 512)
        self.fc3 = nn.Linear(512, nclasses)

    def forward(self, x):
        x = self.pool(F.relu(self.bn1(self.conv1(x))))
        x = self.pool(F.relu(self.bn2(self.conv2(x))))
        x = self.pool(F.relu(self.bn3(self.conv3(x))))
        x = self.pool(F.relu(self.bn4(self.conv4(x))))
        x = x.view(-1, 512 * 2 * 2)
        x = self.dropout(F.relu(self.fc1(x)))
        x = self.dropout(F.relu(self.fc2(x)))
        x = self.fc3(x)
        return F.log_softmax(x, dim=1)

# ======================= 训练与验证函数 =======================
def train(epoch, model, train_loader, optimizer, use_gpu):
    model.train()
    correct = 0
    total_loss = 0
    for batch_idx, (data, target) in enumerate(tqdm(train_loader, desc=f"Train Epoch {epoch}")):
        if use_gpu:
            data, target = data.cuda(), target.cuda()
        optimizer.zero_grad()
        output = model(data)
        loss = F.nll_loss(output, target, reduction='sum')
        loss.backward()
        optimizer.step()
        total_loss += loss.item()
        pred = output.argmax(dim=1, keepdim=True)
        correct += pred.eq(target.view_as(pred)).sum().item()
    avg_loss = total_loss / len(train_loader.dataset)
    accuracy = 100. * correct / len(train_loader.dataset)
    print(f'\nTraining set: Average loss: {avg_loss:.4f}, Accuracy: {correct}/{len(train_loader.dataset)} ({accuracy:.2f}%)')
    return avg_loss, accuracy

def validate(model, val_loader, use_gpu):
    model.eval()
    correct = 0
    total_loss = 0
    with torch.no_grad():
        for data, target in tqdm(val_loader, desc="Validation"):
            if use_gpu:
                data, target = data.cuda(), target.cuda()
            output = model(data)
            loss = F.nll_loss(output, target, reduction='sum')
            total_loss += loss.item()
            pred = output.argmax(dim=1, keepdim=True)
            correct += pred.eq(target.view_as(pred)).sum().item()
    avg_loss = total_loss / len(val_loader.dataset)
    accuracy = 100. * correct / len(val_loader.dataset)
    print(f'Validation set: Average loss: {avg_loss:.4f}, Accuracy: {correct}/{len(val_loader.dataset)} ({accuracy:.2f}%)')
    return avg_loss, accuracy

# ======================= 训练主流程（含早停） =======================
def main():
    # 加载数据
    print("[INFO] Loading data...")
    train_loader, val_loader = processing_data(args.data, args.batch_size, args.use_gpu)
    print(f"[INFO] Train batches: {len(train_loader)}, Val batches: {len(val_loader)}")
    
    # 初始化模型
    model = ImprovedNet(nclasses=43)
    if args.use_gpu:
        model = model.cuda()
    print(model)
    
    # 优化器与学习率调度器
    optimizer = optim.Adam(model.parameters(), lr=args.lr)
    scheduler = optim.lr_scheduler.ReduceLROnPlateau(optimizer, mode='min', patience=5, factor=0.5, verbose=True)
    
    # 记录训练历史
    history = {'loss': [], 'val_loss': [], 'accuracy': [], 'val_accuracy': []}
    
    # 早停相关变量
    best_val_acc = 0.0
    epochs_no_improve = 0
    best_model_state = None
    
    start_time = time.time()
    for epoch in range(1, args.epochs + 1):
        print(f"\n========== Epoch {epoch}/{args.epochs} ==========")
        train_loss, train_acc = train(epoch, model, train_loader, optimizer, args.use_gpu)
        val_loss, val_acc = validate(model, val_loader, args.use_gpu)
        
        # 记录历史
        history['loss'].append(train_loss)
        history['accuracy'].append(train_acc)
        history['val_loss'].append(val_loss)
        history['val_accuracy'].append(val_acc)
        
        # 学习率调度
        scheduler.step(val_loss)
        
        # 早停判断（基于验证准确率）
        if val_acc > best_val_acc + args.min_delta:
            best_val_acc = val_acc
            epochs_no_improve = 0
            best_model_state = model.state_dict()
            # 保存最佳模型
            torch.save(best_model_state, os.path.join(args.results_dir, 'best_model.pth'))
            print(f"*** New best model saved with val_acc: {best_val_acc:.2f}% ***")
        else:
            epochs_no_improve += 1
            print(f"*** No improvement for {epochs_no_improve} epoch(s) (best: {best_val_acc:.2f}%) ***")
        
        # 每个epoch都保存一个副本（可选）
        torch.save(model.state_dict(), os.path.join(args.results_dir, f'model_epoch{epoch}.pth'))
        
        # 早停触发
        if epochs_no_improve >= args.patience:
            print(f"\n[INFO] Early stopping triggered after {epoch} epochs. Best val_acc: {best_val_acc:.2f}%")
            break
    
    # 加载最佳模型（用于最终评估）
    if best_model_state is not None:
        model.load_state_dict(best_model_state)
        print(f"[INFO] Loaded best model with val_acc: {best_val_acc:.2f}%")
    
    # 最终在验证集上评估一次
    final_val_loss, final_val_acc = validate(model, val_loader, args.use_gpu)
    print(f"\n[INFO] Training completed in {time.time()-start_time:.2f} seconds")
    print(f"[INFO] Best validation accuracy: {best_val_acc:.2f}%")
    
    # 保存训练历史
    with open(os.path.join(args.results_dir, 'history.pkl'), 'wb') as f:
        pickle.dump(history, f)
    
    # 绘制学习曲线
    plot_training_history(history, args.results_dir)

def plot_training_history(history, save_dir):
    """绘制损失曲线和准确率曲线"""
    plt.figure(figsize=(12, 5))
    plt.subplot(1, 2, 1)
    plt.plot(history['loss'], label='Train Loss')
    plt.plot(history['val_loss'], label='Val Loss')
    plt.xlabel('Epoch')
    plt.ylabel('Loss')
    plt.legend()
    plt.title('Loss Curves')
    plt.grid(True)
    
    plt.subplot(1, 2, 2)
    plt.plot(history['accuracy'], label='Train Accuracy')
    plt.plot(history['val_accuracy'], label='Val Accuracy')
    plt.xlabel('Epoch')
    plt.ylabel('Accuracy (%)')
    plt.legend()
    plt.title('Accuracy Curves')
    plt.grid(True)
    
    plt.tight_layout()
    plt.savefig(os.path.join(save_dir, 'training_curves.png'), dpi=150)
    plt.show()

if __name__ == '__main__':
    main()