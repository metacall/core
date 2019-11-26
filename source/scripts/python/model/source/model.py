#!/usr/bin/env python3

# Model by: https://github.com/akash-joshi/metacall-ml-example

import os
from joblib import load

current_path = os.path.dirname(os.path.realpath(__file__))

regressor = load(os.path.join(current_path, 'model.joblib'))

def predict_salary(input):
	return regressor.predict(input).tolist()
