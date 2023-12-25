import numpy as np

if __name__ == '__main__':
    # x = np.zeros((100, 100), dtype=np.uint8)
    # print(x)
    # with open('test.npy', 'wb') as f:
    #     np.save(f, x)
    
    with open('a.npy', 'rb') as f:
        a = np.load(f)
    print(a)
    print(a.shape)
