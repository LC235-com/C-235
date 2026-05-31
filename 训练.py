import torch
from torchvision import datasets, transforms
from torch.utils.data import ConcatDataset

def char_order2int_order(charoder):
    """将文件夹名称转换为类别编号（0~42）"""
    try:
        return int(charoder)
    except:
        print(f"Warning: invalid class folder name: {charoder}")
        return -1

# 数据预处理（标准变换）
data_transforms = transforms.Compose([
    transforms.Resize((32, 32)),
    transforms.ToTensor(),
    transforms.Normalize((0.3337, 0.3064, 0.3171), (0.2672, 0.2564, 0.2629))
])

# 数据增强：亮度抖动
data_jitter_brightness = transforms.Compose([
    transforms.Resize((32, 32)),
    transforms.ColorJitter(brightness=0.5),
    transforms.ToTensor(),
    transforms.Normalize((0.3337, 0.3064, 0.3171), (0.2672, 0.2564, 0.2629))
])

def processing_data(data_path, batch_size, use_gpu):
    """加载训练集和验证集，使用数据增强（训练集扩增一倍）"""
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
import torch.nn as nn
import torch.nn.functional as F

class ImprovedNet(nn.Module):
    """改进的CNN网络，包含4个卷积层+BN+Dropout，用于GTSRB分类"""
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
        # 经过4次池化后特征图大小：32 -> 16 -> 8 -> 4 -> 2
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
"训练函数"
def train(epoch, model, train_loader, optimizer, use_gpu):
    """训练一个epoch"""
    model.train()
    correct = 0
    total_loss = 0
    
    for batch_idx, (data, target) in enumerate(train_loader):
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
    print(f'Training set: Average loss: {avg_loss:.4f}, '
          f'Accuracy: {correct}/{len(train_loader.dataset)} ({accuracy:.2f}%)')
    return avg_loss, accuracy
"验证函数"
def validate(model, val_loader, use_gpu):
    """验证模型性能"""
    model.eval()
    correct = 0
    total_loss = 0
    
    with torch.no_grad():
        for data, target in val_loader:
            if use_gpu:
                data, target = data.cuda(), target.cuda()
            output = model(data)
            loss = F.nll_loss(output, target, reduction='sum')
            total_loss += loss.item()
            pred = output.argmax(dim=1, keepdim=True)
            correct += pred.eq(target.view_as(pred)).sum().item()
    
    avg_loss = total_loss / len(val_loader.dataset)
    accuracy = 100. * correct / len(val_loader.dataset)
    print(f'Validation set: Average loss: {avg_loss:.4f}, '
          f'Accuracy: {correct}/{len(val_loader.dataset)} ({accuracy:.2f}%)')
    return avg_loss, accuracy
"超参设置"
import torch
import torch.optim as optim
from torch.optim.lr_scheduler import ReduceLROnPlateau

# ---------- 超参数 ----------
batch_size = 64
epochs = 30
lr = 0.001
seed = 1
data_path = "./data"

# ---------- 随机种子与设备 ----------
torch.manual_seed(seed)
use_gpu = torch.cuda.is_available()
if use_gpu:
    torch.cuda.manual_seed(seed)
    print("使用 GPU 训练")
else:
    print("使用 CPU 训练")

# ---------- 数据加载（假设 processing_data 函数已定义） ----------
train_loader, val_loader = processing_data(data_path, batch_size, use_gpu)

# ---------- 模型初始化 ----------
model = ImprovedNet(nclasses=43)   # 之前定义好的网络
if use_gpu:
    model = model.cuda()

# ---------- 优化器 ----------
optimizer = optim.Adam(model.parameters(), lr=lr)

# ---------- 学习率调度器（当验证损失停滞时降低学习率） ----------
scheduler = ReduceLROnPlateau(optimizer, mode='min', patience=5, factor=0.5, verbose=True)

print("超参数设置完成")
print(f"Batch size: {batch_size}, Epochs: {epochs}, Learning rate: {lr}")