#!coding: utf-8

from svm import *
import os
import time

CRANGE = [1000]
KERNEL_TYPE = [RBF]


for C in CRANGE:
    for KERNEL in KERNEL_TYPE:
        
        print C, KERNEL
        TRAINING_FOLDER = 'Egoshare/DBTraining'
        TEST_FOLDER = 'Egoshare/DBTest'
        VERBOSE = 0
        MODEL_FOLDER = 'Egoshare/Models'
        MODEL_FILE = "model_aligny=0.7_C="+str(C)+"_KERNEL="+str(KERNEL)+".svm"
        GENERATE_ANYWAY = 1

        #G�n�ration du mod�le
        execfile("Train & Test SVM.py")
        
        #Test du mod�le
        execfile("Break_Egoshare_Captcha.py")
        
        time.sleep(1)


raw_input()