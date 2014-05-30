//
//  PTTableCell.m
//  PraatTouch
//
//  Created by Tom Naughton on 1/24/14.
//  Copyright (c) 2014 Tom Naughton. All rights reserved.
//

#import "PTTableCell.h"

@implementation PTTableCell

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
    }
    return self;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

@end
