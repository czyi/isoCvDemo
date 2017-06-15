//
//  MainMenuViewController.m
//  cvdemo
//
//  Created by Ziyi Chen on 6/13/17.
//  Copyright © 2017 Ziyi Chen. All rights reserved.
//

#import "MainMenuViewController.h"
#import "Masonry.h"
#import "ViewController.h"


@interface MainMenuViewController ()<UITableViewDelegate, UITableViewDataSource>

@end

@implementation MainMenuViewController{
    UITableView* tableView;
    NSArray* tableData;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"CV Demo";
    tableView = [UITableView new];
    tableView.dataSource = self;
    tableView.delegate = self;
    [self.view addSubview:tableView];
    
    [tableView mas_remakeConstraints:^(MASConstraintMaker *make) {
        make.edges.equalTo(self.view);
    }];
    

    
    tableData = @[
                  @{
                      @"type": @(CVDemoTypeDefault),
                      @"title": @"Default Demo"
                      },
                  @{
                      @"type": @(CVDemoTypeIncremental),
                      @"title": @"Demo with Incremental Method"
                      },
                  ];
    // Do any additional setup after loading the view.
}


-(NSInteger)numberOfSectionsInTableView:(UITableView *)tableView{
    return 1;
}

-(NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section{
    return tableData.count;
}

-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath{
    return 50;
}

-(UITableViewCell *)tableView:(UITableView *)_tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath{
    UITableViewCell* cell = [_tableView dequeueReusableCellWithIdentifier:@"cell"];
    if(cell == nil){
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"cell"];
    }
    [cell.textLabel setText:[[tableData objectAtIndex:indexPath.row] valueForKey:@"title"]];
    return cell;
}


-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath{
    ViewController* viewController = [ViewController new];
    viewController.title = [[tableData objectAtIndex:indexPath.row] valueForKey:@"title"];
    viewController.demoType = [[[tableData objectAtIndex:indexPath.row] valueForKey:@"type"] intValue];
    [self.navigationController pushViewController:viewController animated:YES];
}



/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
