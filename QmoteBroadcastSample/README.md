Android Broadcast Receiver Sample APP
==============

This sample code will help developer to quick integrate your app to work with Qmote app by simply receiving the broadcast intent sent from Qblinks Qmote Android App.

Since Qmote Android App **version 5.0.3**(“QmoteClick” only)/**version 5.0.4**(“QmoteClick” and “QmoteAddress”), it sends broadcast message for every Qmote activities. You can implement broadcast message receiver in your app, and your app will work with Qmote nicely.


###Broadcast Intent Format

Action Name: “QmoteCommand”<br />
Key:   “QmoteClick”<br />
Value: <br />
       “s”     as a single short click<br />
       “ss”    as short-short<br />
       “sss”   as short-short-short<br />
       “ssss”  as short-short-short-short<br />
       “ls”    as long-shortas<br />
       “lss”   as long-short-short<br />
       “lsss”  as long-short-short-short<br />
       “lls”   as long-long-short<br />
       “sls”   as short-long-short<br />
<br />
Key: “QmoteAddress”<br />
Value:<br />
      Standtard Address such as “54:4A:16:77:C9:2F”<br />

###Get Qmote List

1. Function getQmote() will get Qmote addresses in Bluetooth system.
Then you can know the addresses you expected to see in your receiver.

2. Include a BroadcastReceiver to receive Qmote click.
