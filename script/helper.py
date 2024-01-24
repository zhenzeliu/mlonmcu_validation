import numpy as np
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

if __name__ == '__main__':
    npy_visualize("out.npy")