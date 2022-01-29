import numpy as np 
import cv2 
import pytesseract

def image_resize(image, width = None, height = None, inter = cv2.INTER_AREA):
    # initialize the dimensions of the image to be resized and
    # grab the image size
    dim = None
    (h, w) = image.shape[:2]

    # if both the width and height are None, then return the
    # original image
    if width is None and height is None:
        return image

    # check to see if the width is None
    if width is None:
        # calculate the ratio of the height and construct the
        # dimensions
        r = height / float(h)
        dim = (int(w * r), height)

    # otherwise, the height is None
    else:
        # calculate the ratio of the width and construct the
        # dimensions
        r = width / float(w)
        dim = (width, int(h * r))

    # resize the image
    resized = cv2.resize(image, dim, interpolation = inter)

    # return the resized image
    return resized

def run():
    img = cv2.imread('1.jpg') 
    pytesseract.pytesseract.tesseract_cmd = r'C:\Users\uic54825\AppData\Local\Programs\Tesseract-OCR\tesseract.exe' 
    
    
    #img = image_resize(img, height = 600)
    
    y = 2100
    h = 150
    x = 950
    w = 630
    crop_img = img[y:y+h, x:x+w]
    gray = cv2.cvtColor(crop_img, cv2.COLOR_BGR2GRAY) 
    
    imagem = (255-gray)
    ret,thresh = cv2.threshold(imagem,100,200,1) 
    '''
    contours,h = cv2.findContours(thresh,1,2)

    for cnt in contours: 
        approx = cv2.approxPolyDP(cnt,0.01*cv2.arcLength(cnt,True),True) 
        print (len(approx))
        if len(approx)==4: 
            cv2.drawContours(img,[cnt],0,(0,0,255),-1) 
    '''
    
    blurred = cv2.GaussianBlur(gray, (3, 3), 0)    
    edged = cv2.Canny(blurred, 50, 200, 255)
    #cv2.imshow('thresh', thresh)  
    cv2.imshow('img', crop_img) 
    cv2.imshow('edged', gray) 
    cv2.imshow('edged', thresh)
    print(pytesseract.image_to_string(thresh,config=' --psm 1 --oem 3  -c tessedit_char_whitelist=0123456789/'))
    cv2.waitKey(0) 
    cv2.destroyAllWindows()
    
    



if __name__=="__main__":
    run()


