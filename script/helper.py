import numpy as np
import subprocess

import os

def helper_readout_npy(file: str):
    if type(file) is not str:
        raise ValueError("Input type is not str.");

    if not os.path.exists(file):
        raise ValueError("File does not exist.")

    with open(file, 'rb') as fp:
        res = np.load(fp)
    print('data:', res)
    print('shape:', res.shape)
    print('dtype:', res.dtype)

def helper_readout_stream(proc: str):
    res = subprocess.run(proc, capture_output=True, text=True)
    return res.stdout.split('\n')

if __name__ == '__main__':
    # helper_readout_npy('/home/zhenze/mlonmcu_validation/a.npy')
    print(helper_readout_stream("./test"))
