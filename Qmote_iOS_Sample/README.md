iOS Sample app
==============
This sample code will help developer integrate self app with Qmote hardware.
In this app, you will learn how to connect, write command and read device info with Qmote.
With this app, the original Qblinks Qmote app could use at the same time, it won't affect connection to each other.

Before use this app, you need go to iOS system setting/bluetooth scan and pair a Qmote. If your Qmote has already paired in the list, you could ignore this note.

Qmote GATT specification you could download [here](http://qblinks.com/devkit/developers/qmote-developers).
All command codes below you could find them in this document, please read it first.

### Screenshot
![App screenshot](https://github.com/qblinks/qmote_makers_projects/blob/master/Qmote_iOS_Sample/Screenshot.PNG?raw=true)

Let's Start
===========
### Connect Qmote
This button will scan the system setting list and get the first Qmote which has already connected in system.
If you have many Qmote, please use a NSArray to maintain and get the specific Qmote by index number.

### Click label text
When you press the Qmote button, this label will show your press pattern and code.
Qmote does not sense long-click until a long-click function is added. That is to reduce long/short mis-judge if there isn't any long-click feature added. So in this sample app, we forced to send a long press command code to Qmote in Send_CMD2_Qmote() function.

### Keep-alive
If user doesn't press Qmote for a few seconds, Qmote will into sleep mode to save power and Qmote will disconnect from iPhone. This keep-alive command will extend the time. If you want Qmote keep waking up, we suggest that send this command every 30 seconds to Qmote in foreground.

### Get FW version
This function will help you learn how would get the Qmote FW version. Send a request command to Qmote and you will get the return value at QPS_Q1_CB_UUID characteristic. 
