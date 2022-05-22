import smtplib

def SendEmailFunction():
    sender = ""   
    sender_pswrd=""
    receiver = []

    with open("senderemail.txt") as f2: #put your sender email address and sender email password in this txt file
        lines2 = f2.read().splitlines()
    sender=lines2[0]
    sender_pswrd=lines2[1]

    with open("receiver_email_config.txt") as f:        # read receiver_email_config.txt file for receiver email
        lines = f.read().splitlines()
    for line in lines:
        receiver.append(line)

    server = smtplib.SMTP_SSL('smtp.gmail.com', 465)    #use smtplib library to send email
    server.login(sender, sender_pswrd)
    status=server.sendmail(sender, receiver, "fall motion detected")
    if status == {}:
        print(receiver,': Success!')
    else:
        print(receiver,': Fail!')
    server.quit()