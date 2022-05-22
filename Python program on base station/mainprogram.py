import asyncio
import time
from bleak import BleakClient
from datetime import datetime
import pandas as pd
from selenium import webdriver
import os

from sendemail import SendEmailFunction

address = "48:AA:40:50:64:4D"   #bluetooth address of the Arduino Board
MODEL_NBR_UUID = "50b837d1-db86-4d6d-bb4d-e0b725ba0932" #the UUID can also be configured on Arduino Code, just need to be matched for both sides on Arduino and python.


currentDIR=os.getcwd()
URL=currentDIR+"/image/loading.gif" #URL variables are the gif or image files that will be opened by chrome driver to show illustration images or gifs.


driver = webdriver.Chrome('./chromedriver')     #set chrome driver
driver.set_window_position(0, 0)
driver.set_window_size(512, 400)
driver.get(URL) #use chrome driver to show gif 

async def main(address):
    labelNew="start"    #store the new motion prediction status
    labelOld="start"    #store the old motion prediction status. If old==new, don't refresh chrome driver.
    async with BleakClient(address) as client:
        while(True):
            date_time=datetime.now().strftime("%m/%d/%Y_%H:%M:%S")
            x = await client.read_gatt_char(MODEL_NBR_UUID) #get bluetooth signal from arduino board
            print (x.decode())
            pd.DataFrame({'Time':[date_time],'Label':[x.decode()]}).to_csv("record.csv", mode='a', index=False, header=False) #store data in appendedfile.csv file
            if("fall" in x.decode()):
                print("fall detected")
                labelOld=labelNew
                labelNew="fall"
                URL=currentDIR+"/image/fall.gif"
                driver.get(URL)  #use chrome driver to show gif 
                SendEmailFunction() #send email
            elif("idle" in x.decode()):
                labelOld=labelNew
                labelNew="idle"
                URL=currentDIR+"/image/idle.png"
            elif("normal_walk" in x.decode()):
                labelOld=labelNew
                labelNew="normal_walk"
                URL=currentDIR+"/image/normal_walk.gif"
            elif("parkinsonian_gait" in x.decode()):
                labelOld=labelNew
                labelNew="parkinsonian_gait"
                URL=currentDIR+"/image/parkinsonian_gait.gif"
            elif("slap_gait" in x.decode()):
                labelOld=labelNew
                labelNew="slap_gait"
                URL=currentDIR+"/image/slap_gait.gif"
            elif("antalgic_gait" in x.decode()):
                labelOld=labelNew
                labelNew="antalgic_gait"
                URL=currentDIR+"/image/antalgic_gait.gif"
            elif("ataxic_gait" in x.decode()):
                labelOld=labelNew
                labelNew="ataxic_gait"
                URL=currentDIR+"/image/ataxic_gait.gif"
            elif("diplegic_gait" in x.decode()):
                labelOld=labelNew
                labelNew="diplegic_gait"
                URL=currentDIR+"/image/diplegic_gait.gif"
            
            if(labelNew!=labelOld and labelNew!="fall"): #If old!=new, refresh chrome driver to show gif of the new prediction result.
                driver.get(URL)
            

while(True):
    try:
        asyncio.run(main(address))
    except Exception as e:
        print(e)




