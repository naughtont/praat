//
//  PTTableViewController.h
//  PraatTouch
//
//  Created by Tom Naughton on 1/24/14.
//  Copyright (c) 2014 Tom Naughton. All rights reserved.
//

#import <UIKit/UIKit.h>

@class PTViewController;

@interface PTTableViewController : UITableViewController
@property (strong, nonatomic) NSMutableArray *viewControllers;

- (void)registerViewController:(PTViewController*)viewController;

@end
