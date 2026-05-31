import os

def initialize_data(folder):
    """将每个类中的部分图片从训练集移动到验证集"""
    train_folder = os.path.join(folder, 'train_images')
    val_folder = os.path.join(folder, 'val_images')

    # 如果 val_images 文件夹不存在，就创建它
    if not os.path.isdir(val_folder):
        print(f'{val_folder} not found, making a validation set')
        os.makedirs(val_folder, exist_ok=True)

        # 遍历 train_images 下的每个类别文件夹 (0, 1, 2, ...)
        for class_dir in os.listdir(train_folder):
            class_path = os.path.join(train_folder, class_dir)
            if not os.path.isdir(class_path):
                continue

            # 在 val_images 下创建对应的类别文件夹
            val_class_path = os.path.join(val_folder, class_dir)
            os.makedirs(val_class_path, exist_ok=True)

            # 遍历该类下的所有图片
            for f in os.listdir(class_path):
                # 检查文件名第7到12位（索引6到11）是否符合规则
                if len(f) > 11 and f[6:11] in ('00000', '00001', '00002'):
                    # 构建完整的源文件和目标文件路径
                    src = os.path.join(class_path, f)
                    dst = os.path.join(val_class_path, f)
                    # 移动文件
                    os.rename(src, dst)

    print("数据集划分完成！")
if __name__ == '__main__':
    data_path = "./data"  # 你的数据根目录
    initialize_data(data_path)
    print("数据预处理完成。")