Android Sample app
==============
This sample code will help developer to create a self-app of Qmote hardware.
In this app, you will learn how to connect, write command and read device info with Qmote.
The original Qblinks Qmote app could be used at the same time while you use this app. It won't affect each other’s connection.

Before using this app, you need to go to Android Settings -> Bluetooth -> pair a Qmote. If your Qmote has already paired in the list, you could ignore this note.

Qmote GATT specification you could download [here](http://qblinks.com/devkit/developers/qmote-developers).
All command codes below you could find them in this document, please read it first.

### Screenshot
![Screenshot_Android](https://github.com/qblinks/qmote_makers_projects/blob/master/Qmote_Android_Sample/Screenshot_Android.jpg?raw=true)

Let's Start
===========
### Connect Qmote
This button will scan the paired devices list and get the first Qmote which has already connected in system.
If you have many Qmotes, please use a Set<BluetoothDevice> to maintain and get the specific Qmote by index number.
When the connection state change to “connected”, it will start to discoverServices and initial setting of the Qmote(Including set notification on and enable long-click of Qmote).

### Click event text
When you press the Qmote button, this label will show your press pattern and code. It will also show the connection state when the connection state change.
PS. Qmote does not sense long-click until a long-click function is added. That is to reduce long/short mis-judge if there isn't any long-click feature added. So in this sample app, we forced to send a long press command code to Qmote in initial setting.

### Keep-alive
If user doesn't press the Qmote for a few seconds, the Qmote will be in sleep mode for saving power and disconnect with the Android device. This keep-alive command will extend the connection time. If you want the Qmote to stay awake, we suggest that send this command every 30 seconds to Qmote in the foreground.

### Get FW version
This function will help you learn how would get the Qmote FW version. Send a request command to UUID_COMMAND_CHARACTERISTIC and you will get the return value at UUID_CALLBACK_CHARACTERISTIC. 
