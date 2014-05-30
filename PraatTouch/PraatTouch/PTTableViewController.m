//
//  PTTableViewController.m
//  PraatTouch
//
//  Created by Tom Naughton on 1/24/14.
//  Copyright (c) 2014 Tom Naughton. All rights reserved.
//

#import "PTTableViewController.h"
#import "PTViewController.h"
#import "PTTableCell.h"

@interface PTTableViewController ()

@end

@implementation PTTableViewController

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.viewControllers count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"tableCell";
    PTTableCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
    
    // Configure the cell...
    PTViewController *viewController = self.viewControllers[indexPath.row];
    cell.label.text = viewController.title;
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    UIViewController *vc = self.viewControllers[indexPath.row];
    [self.navigationController pushViewController:vc animated:YES];
}


- (void)registerViewController:(PTViewController*)viewController
{
    if (!self.viewControllers) {
        self.viewControllers = [[NSMutableArray alloc] init];
    }

    [self.viewControllers addObject:viewController];
    [self.tableView reloadData];
    [self.navigationController pushViewController:viewController animated:YES];

}



@end
