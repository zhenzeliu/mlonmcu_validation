import numpy as np
import yaml
import os

# load data from npy file
def load_npy(path: str) -> np.ndarray:
    if type(path) is not str:
        raise TypeError("Input type is not string.")
    if not os.path.exists(path):
        raise FileNotFoundError("File does not exist.")
    _, ext = os.path.splitext(path)
    if ext != ".npy":
        raise ValueError("File must be .npy format.")
    
    with open(path, 'rb') as fp:
        res = np.load(fp)
    return np.frombuffer(res.tobytes(), dtype=np.int8)

# load data from binary file
def load_bin(path: str) -> np.ndarray:
    if type(path) is not str:
        raise TypeError("Input type is not string.")
    if not os.path.exists(path):
        raise FileNotFoundError("File does not exist.")
    _, ext = os.path.splitext(path)
    if ext != ".bin":
        raise ValueError("File must be .bin format.")
    
    with open(path, 'rb') as fp:
        res = fp.read()
    return np.frombuffer(res, dtype=np.int8)

class Validator:

    def __init__(self, model_info_path: str) -> None:
        self.tol = None
        self.model_info = None
        self.load_model(model_info_path)
    
    # load model
    def load_model(self, model_info_path: str) -> None:
        if not os.path.exists(model_info_path):
            raise FileNotFoundError("File does not exist.")
        with open(model_info_path, 'r') as fp:
            model_info = yaml.safe_load(fp)
        self.model_info = model_info
    
    # dequantization
    def dequantize(self, input:np.ndarray) -> np.ndarray:
        model_info = self.model_info
        if model_info == None:
            raise ValueError("Model info has not been loaded yet.")
        coeff = model_info["output_quant_details"][0][0]
        zero_pt = model_info["output_quant_details"][0][1]
        return coeff * (input - zero_pt)

    # bit-exactly equal
    def array_equal(self, outputs_1: np.ndarray, outputs_2: np.ndarray) -> bool:
        if outputs_1.shape != outputs_2.shape:
            return False
        return np.array_equal(outputs_1, outputs_2)
    
    # closeness of 2 arrays with tolerance
    # abs(a-b) <= (atol + rtol * abs(b))
    def array_close(self, outputs_1: np.ndarray, outputs_2: np.ndarray, rtol: float, atol: float, thresh: int):
        if outputs_1.shape != outputs_2.shape:
            return False
        compared_res = np.isclose(outputs_1, outputs_2, rtol, atol).astype(int)
        cnt = np.add.reduce(compared_res)
        return True if cnt >= thresh else False

    # euclidean distance
    def dist_euclidean(self, outputs_1: np.ndarray, outputs_2: np.ndarray, maxdist: float):
        if outputs_1.shape != outputs_2.shape:
            return False
        dist = np.linalg.norm(outputs_1 - outputs_2)
        return True if dist <= maxdist else False
    
    # cityblock distance
    def dis_cityblock(self, outputs_1: np.ndarray, outputs_2: np.ndarray, maxdist: float):
        if outputs_1.shape != outputs_2.shape:
            return False
        dist = np.abs(outputs_1, outputs_2).sum()
        return True if dist <= maxdist else False

    # cosine similarity
    def sim_cosine(self, outputs_1: np.ndarray, outputs_2: np.ndarray, minsim: float):
        if outputs_1.shape != outputs_2.shape:
            return False
        sim = np.dot(outputs_1, outputs_2) / (np.linalg.norm(outputs_1) * np.linalg.norm(outputs_2))
        return True if sim >= minsim else False

if __name__ == "__main__":
    v = Validator("model_info.yml")
    data = load_bin("out.bin")
    data_f = v.dequantize(data)

    data_npy = load_npy("out.npy")
    data_npy_f = v.dequantize(data_npy)
    # res = v.array_equal(data_f, data_npy_f)
    res = v.array_close(data_f, data_npy_f, 1e-05, 1e-08, 10)
    print(res)
    res = v.sim_cosine(data_f, data_npy_f, 1e-02)
    print(res)
