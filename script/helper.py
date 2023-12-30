import numpy as np
import subprocess
import os

def npy_visualize(file: str):
    if type(file) is not str:
        raise ValueError("Input type is not str.")
    if not os.path.exists(file):
        raise ValueError("File does not exist.")

    with open(file, 'rb') as fp:
        res = np.load(fp)
    print(f"Opened file: {file}")
    print("Data:")
    print(res)
    print(f"Shape: {res.shape}, Dtype: {res.dtype}")

# verification
def verify(model_output: bytes, reference_output: bytes, model_output_size: list, reference_output_size: list) -> bool:
    if len(model_output_size) != len(reference_output_size):
        raise ValueError("Model output shape does not match reference shape.")
    if model_output_size != reference_output_size:
        raise ValueError("Model output size does not match reference size.")
    if model_output == reference_output:
        return True
    else:
        return False
    

def pipeline_stdout(proc: str):
    res = subprocess.run(proc, capture_output=True, text=True)
    return res.stdout.split('\n')

if __name__ == '__main__':
    # npy_visualize('/home/zhenze/mlonmcu_validation/a.npy')
    print(pipeline_stdout("./test"))
    a = b'\x6d\x11'
    b = b'\x6e\x11'
    print(verify(a, b, [2], [2]))
