# EmbeddedMachineLearning_DetectAbnormalGait
 This is a project of building a device that can detect user's abnormal gaits and send alert emails when falling motion detected.
<br />
 Demo Video: https://www.youtube.com/watch?v=o6Hw3dhA_e8
 <br />
 The manuscript of the device is in the DetectFalling&AbnormalGait_Manuscript.pdf file.
<br /><br />
My device is designed to solve the problem that, elderly or some patients have higher risk of falling over due to the lack of muscle strength. There might not be a person to look after the elderly or
patient all the time, and the consequences of falling over might be serious if not treated immediately after the falling incident. As a result, I would like to propose a device which can track user’s gait
motion and raise alert when user falling over. There are two main functions of my device: 1) detect user’s abnormal gaits and record the walking motions. 2) Send alert emails when falling motion
detected. The system is implemented using Arduino Nano 33 Sense, Edge Impulse, Bluetooth, and Python.
<br /><br />
<h2>System design</h2>
<br />
<img src="images/diagram.png" width="500px">
<br />
There are two key features of my device, one is to recognize abnormal
gaits, the other one is to raise warning when falling motion
detected. There are four important steps in my progress of building
the system. 1) Data collection 2) Machine Learning Model Building
and Training 3) Integrating Bluetooth function with the machine
learning model generated from Edge Impulse 4) Build a python
program on base station.
<br />
<h2>Data collection</h2>
Plot of all extracted features from data:
<br />
<img src="images/data feature.png" width="500px">
<br />
There are 8 classes of data collected: idle, fall, normal walking,
parkinsonian gait, slap gait, antalgic gait, diplegic gait, ataxic gait.
I have totally collected 3 hours of data, with each class having
approximately 20 minutes of data.
<h2>Model Architecture and Training</h2>
Diagram of the model architecture:
<br />
<img src="images/model architecture.png" width="500px">
<br />
My model architecture had
3 layers of dense layers, with 20, 10 and 10 neurons respectively.
The model was trained in 50 epochs, with the learning rate of 0.0005.
The validation data was split from training data, with a portion
of 20 %.
<br />
<h2>Accuracy</h2>
<br />
<img src="images/train result.png" width="500px">
<br />
The result of the trained model achieved an accuracy of
94.7 %, and all classes of data have accuracy over 90 %.
<br />
<h2>Power Consumption</h2>
<br />
<img src="images/pwr consumption.png" width="500px">
<br />
The result of the power consumption experiment showed that the system can operate
normally (continue to predict motion and transmit prediction result
to a base station via Bluetooth), for over 20 hours using
Amazon 9v alkaline battery.
