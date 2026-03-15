import random

def Generate_Random_Number(num_words, filename):
    """Generate number with base 2^32 and saves it into file as a line, 
    where each word is space-seperated"""
    words=[]
    
    while True:
        #First Most Significat Word is not zero!
        word = random.getrandbits(32)
        if word != 0:
            words.append(str(word))
            break


    for _ in range(num_words-1):
        word = random.getrandbits(32)
        words.append(str(word))
    
    line = " ".join(words)

    with open(filename,'a') as f:
        f.write(line + "\n")




# MAIN

for i in range(50,1000+1,50):
    filename = 'data_{}.txt'.format(i)
    
    for _ in range(0,100):
        Generate_Random_Number(i, filename)