/*
 * Copyright (c) 2016
 * Qblinks Corporation.
 * All rights reserved.
 *
 * The information contained herein is confidential and proprietary to
 * Qblinks. Use of this information by anyone other than authorized employees
 * of Qblinks is granted only under a written non-disclosure agreement,
 * expressly prescribing the scope and manner of such use.
 *
 */

#import <UIKit/UIKit.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <CoreBluetooth/CBService.h>

#define QPS_Q1_SERVICE_UUID     @"E8008802-4143-5453-5162-6C696E6B73EC"
#define QPS_Q1_CMD_UUID         @"E8009A01-4143-5453-5162-6C696E6B73EC"
#define QPS_Q1_CB_UUID          @"E8009A02-4143-5453-5162-6C696E6B73EC"
#define QPS_Q1_BTN_UUID         @"E8009A03-4143-5453-5162-6C696E6B73EC"
#define F_APP_DEF           0x01

@interface ViewController : UIViewController<CBCentralManagerDelegate, CBPeripheralDelegate>
{
@private
    CBUUID *_QPS_ServiceUUID;
    CBUUID *_QPS_CMD_CharacterUUID;
    CBUUID *_QPS_CB_CharacterUUID;
    CBUUID *_QPS_BTN_CharacterUUID;
    
    NSTimer *scan_timer;
}

@property (weak, nonatomic) IBOutlet UIButton *connect_btn;
@property (weak, nonatomic) NSArray *Qmote_list;
@property (strong, nonatomic) CBCentralManager *CM;
@property (strong, nonatomic) CBPeripheral *Qmote_p;

@property (strong, nonatomic) IBOutlet UIButton *scan_btn;
- (IBAction)scan_btn_click:(id)sender;

- (IBAction)connect_btn_click:(id)sender;
@property (weak, nonatomic) IBOutlet UILabel *click_label;
@property (weak, nonatomic) IBOutlet UILabel *btn_release;

@property (weak, nonatomic) IBOutlet UIButton *keepalive_btn;
- (IBAction)keepalive_touch:(id)sender;

@property (weak, nonatomic) IBOutlet UIButton *fw_version_btn;
- (IBAction)fw_version_touch:(id)sender;

@property (strong, nonatomic) NSMutableArray *peripherals;

@end

