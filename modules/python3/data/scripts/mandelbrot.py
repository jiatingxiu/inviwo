import inviwopy
from inviwopy.glm import *
import numpy as np
import math

# input variables 
# img - memory for the final image
# real - real bounds of the image
# im -  imaginary bound of the image
# power - the power in the function Z_{n+1} = Z_n + C^{power}
# N - number of iterations

print(type(img))
print(dir(img))

#data = np.array(img, copy=False)

data = img.data

print(type(data))
print(data.shape)

realRange = real[1] - real[0]
imRange = im[1] - im[0]



dimx = data.shape[0];
dimy = data.shape[1];

size = data.shape 

#x and y are in pixel coordinates 
def toImaginary( x,y ):
    r  = x / (size[0]-1)
    i  = y / (size[1]-1)
    r = real[0] + r * realRange;
    i = im[0]   + i * imRange;
    
    return complex(r,i);

def toScalar(Z,i):
    x = math.log(1+i)
    return x

counter = 0
if True: 
    for y in range(0,size[1]):
        for x in range(0,size[0]):
            C = Z = toImaginary(x,y);
            for i in range(0,N):
                if(abs(Z)>2):
                    data[x,y] = toScalar(Z,i);
                    break;
                Z = np.power(Z,power) + C;