/*
ViewController.mm

A view controller for the TestGL demo program.
*/

#import "ViewController.h"
#import <CartoType/CartoType.h>
#import <vector>

@implementation ViewController

// instance variables
{
CartoTypeFramework* m_framework;
int m_ui_scale;
CartoTypePoint m_route_start_in_degrees;
CartoTypePoint m_route_end_in_degrees;
CartoTypePoint m_last_point_pressed_in_degrees;
UISearchBar* m_search_bar;
UIButton* m_navigate_button;
uint64_t m_pushpin_id;
bool m_navigating;
CLLocationManager* m_location_manager;
CLLocation* m_location;
}

-(id)init:(CartoTypeFramework*)aFramework bounds:(CGRect)aBounds
    {
    if (aFramework == nil)
        return nil;
    if (!(self = [super init:aFramework bounds:aBounds]))
        return nil;
    m_framework = aFramework;

    [self becomeFirstResponder];
    m_ui_scale = [[UIScreen mainScreen] scale];
    
    // Create a search bar.
    m_search_bar = [[UISearchBar alloc] init];
    m_search_bar.delegate = self;
    m_search_bar.frame = CGRectMake(0, 0, 300, 40);
    m_search_bar.layer.position = CGPointMake(self.view.bounds.size.width / 2,40);
    
    // add shadow
    //mySearchBar.layer.shadowColor = UIColor.blackColor().CGColor
    //mySearchBar.layer.shadowOpacity = 0.5
    //mySearchBar.layer.masksToBounds = false
    
    // show cancel button
    m_search_bar.showsCancelButton = true;
    
    // hide bookmark button
    //mySearchBar.showsBookmarkButton = false
    
    // set Default bar status.
    //mySearchBar.searchBarStyle = UISearchBarStyle.Default
    
    // set title
    //mySearchBar.prompt = "Title"
    
    // set placeholder
    m_search_bar.placeholder = @"place name";
    
    // change the color of cursol and cancel button.
    //mySearchBar.tintColor = UIColor.redColor()
    
    // hide the search result.
    //mySearchBar.showsSearchResultsButton = false
    
    // add searchBar to the view.
    [self.view addSubview:m_search_bar];
    
    // Create a button to start and stop navigating.
    m_navigate_button = [[UIButton alloc] init];
    m_navigate_button.frame = { 0,0,50,25 };
    m_navigate_button.layer.position = { self.view.bounds.size.width / 2,self.view.bounds.size.height - 30 };
    [m_navigate_button setTitleColor:UIColor.redColor forState:UIControlStateNormal];
    m_navigate_button.backgroundColor = UIColor.whiteColor;
    [m_navigate_button setTitle:@"Start" forState:UIControlStateNormal];
    [m_navigate_button addTarget:self action:@selector(startOrEndNavigation) forControlEvents:UIControlEventTouchUpInside];
    m_navigate_button.hidden = true;
    [self.view addSubview:m_navigate_button];
    
    // set up location services
    m_location_manager = [[CLLocationManager alloc] init];
    m_location_manager.delegate = self;
    m_location_manager.desiredAccuracy = kCLLocationAccuracyBest;
    
    if ([CLLocationManager authorizationStatus] != kCLAuthorizationStatusAuthorizedAlways &&
        [CLLocationManager authorizationStatus] != kCLAuthorizationStatusAuthorizedWhenInUse)
        [m_location_manager requestWhenInUseAuthorization];
    
    // Set the vehicle position to a quarter of the way up the display.
    [m_framework setVehiclePosOffsetX:0 andY:0.25];
    
    // Display turn instructions.
    [m_framework setTurnInstructions:false abbreviate:true width:1.5 widthUnit:"in" position:NoticePositionTopLeft textSize:12 textSizeUnit:"pt"];
    
    // Uncomment the following line to set the routing profile to hiking.
    // [m_framework setMainProfileType:HikingProfile];
    
    return self;
    }

-(void)viewWillTransitionToSize:(CGSize)aSize withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)aCoordinator
    {
    [super viewWillTransitionToSize:aSize withTransitionCoordinator:aCoordinator];
    m_navigate_button.layer.position = { aSize.width / 2,aSize.height - 30 };
    }

-(void)onTap:(CartoTypePoint)aPointInDegrees
    {
    // toggle perspective mode
    [m_framework setPerspective:![m_framework getPerspective]];
    }

-(void)onLongPress:(CartoTypePoint)aPointInDegrees
    {
    m_last_point_pressed_in_degrees = aPointInDegrees;
    CartoTypePoint p = m_last_point_pressed_in_degrees;
    [m_framework convertPoint:&p from:DegreeCoordType to:DisplayCoordType];
    
    // Find nearby objects.
    NSMutableArray* object_array = [[NSMutableArray alloc] init];
    double pixel_mm = [m_framework getResolutionDpi] / 25.4;
    [m_framework findInDisplay:object_array maxItems:10 point:p radius:ceil(2 * pixel_mm)];
    
    // See if we have a pushpin.
    m_pushpin_id = 0;
    for (int i = 0; i < [object_array count]; i++)
        {
        CartoTypeMapObject* cur_object = (CartoTypeMapObject*)[object_array objectAtIndex:i];
        if ([[cur_object getLayerName]  isEqual: @"pushpin"])
            {
            m_pushpin_id = [cur_object getObjectId];
            break;
            }
        }
    
    // Create the menu.
    UIMenuController* menu = [UIMenuController sharedMenuController];
    
    UIMenuItem* pushpin_menu_item;
    if (m_pushpin_id)
        pushpin_menu_item = [[UIMenuItem alloc] initWithTitle:@"Delete pin" action:@selector(deletePushPin)];
    else
        pushpin_menu_item = [[UIMenuItem alloc] initWithTitle:@"Insert pin" action:@selector(insertPushPin)];
    
    menu.menuItems =
    @[
      pushpin_menu_item,
      [[UIMenuItem alloc] initWithTitle:@"Start here" action:@selector(setRouteStart)],
      [[UIMenuItem alloc] initWithTitle:@"End here" action:@selector(setRouteEnd)]
      ];
    
    CGRect target = CGRectMake(p.x / m_ui_scale,p.y / m_ui_scale,1,1);
    [menu setTargetRect:target inView:self.view];
    [menu setMenuVisible:YES animated:YES];
    }

-(void)setRouteStart
    {
    m_route_start_in_degrees = m_last_point_pressed_in_degrees;
    if (m_route_end_in_degrees.x && m_route_end_in_degrees.y)
        {
        m_navigate_button.hidden = false;
        [m_framework startNavigationFrom:m_route_start_in_degrees startCoordType:DegreeCoordType to:m_route_end_in_degrees endCoordType:DegreeCoordType];
        }
    }

-(void)setRouteEnd
    {
    m_route_end_in_degrees = m_last_point_pressed_in_degrees;
    if (m_route_start_in_degrees.x && m_route_start_in_degrees.y)
        [m_framework startNavigationFrom:m_route_start_in_degrees startCoordType:DegreeCoordType to:m_route_end_in_degrees endCoordType:DegreeCoordType];
    m_navigate_button.hidden = false;
    }

-(void)insertPushPin
    {
    CartoTypeAddress* a = [[CartoTypeAddress alloc] init];
    [m_framework getAddress:a point:m_last_point_pressed_in_degrees coordType:DegreeCoordType];
    CartoTypeMapObjectParam* p = [[CartoTypeMapObjectParam alloc] initWithType:PointMapObjectType andLayer:@"pushpin" andCoordType:DegreeCoordType];
    [p appendX:m_last_point_pressed_in_degrees.x andY:m_last_point_pressed_in_degrees.y];
    p.mapHandle = 0;
    p.stringAttrib = [a ToString:false];
    [m_framework insertMapObject:p];
    m_pushpin_id = p.objectId;
    }

-(void)deletePushPin
    {
    [m_framework deleteObjectsFromMap:0 fromID:m_pushpin_id toID:m_pushpin_id withCondition:nullptr andCount:nullptr];
    m_pushpin_id = 0;
    }

-(void)startOrEndNavigation
    {
    if (!m_navigating)
        {
        [m_location_manager startUpdatingLocation];
        [UIApplication sharedApplication].idleTimerDisabled = YES;
        
        // Try to get start of route if not known.
        if (![m_framework getRouteCount])
            {
            if (m_location)
                {
                m_route_start_in_degrees.x = m_location.coordinate.longitude;
                m_route_start_in_degrees.y = m_location.coordinate.latitude;
                [m_framework startNavigationFrom:m_route_start_in_degrees startCoordType:DegreeCoordType to:m_route_end_in_degrees endCoordType:DegreeCoordType];
                }
            }
        
        if ([m_framework getRouteCount])
            {
            [m_navigate_button setTitle:@"End" forState:UIControlStateNormal];
            m_search_bar.hidden = true;
            [m_framework enableTurnInstructions:true];
            m_navigating = true;
            }
        
        }
    else
        {
        [m_navigate_button setTitle:@"Start" forState:UIControlStateNormal];
        m_search_bar.hidden = false;
        [m_framework enableTurnInstructions:false];
        m_navigating = false;
        m_route_start_in_degrees = { 0, 0 };
        m_route_end_in_degrees = { 0, 0 };
        [m_framework deleteRoutes];
        m_navigate_button.hidden = true;
        }

    if (!m_navigating)
        {
        [m_location_manager stopUpdatingLocation];
        [UIApplication sharedApplication].idleTimerDisabled = NO;
        }
    }

-(void)didReceiveMemoryWarning
    {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
    }

// Prevent the search bar from getting touch events so that tapping in it doesn't change the route.
-(BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)aTouch
    {
    if ([aTouch.view isDescendantOfView:m_search_bar])
        return NO;
    return YES;
    }

-(void)searchBar:(UISearchBar *)searchBar textDidChange:(NSString *)aSearchText
    {
    }

-(void)searchBarCancelButtonClicked:(UISearchBar *)aSearchBar
    {
    [self.view endEditing:YES];
    }

-(void)searchBarSearchButtonClicked:(UISearchBar *)aSearchBar
    {
    [self.view endEditing:YES];
    NSString* text = [aSearchBar text];
    if (text)
        {
        NSMutableArray* found = [[NSMutableArray alloc] init];
        CartoTypeFindParam* param = [[CartoTypeFindParam alloc] init];
        param.text = text;
        [m_framework find:found withParam:param];
        if ([found count] > 0)
            {
            CartoTypeMapObject* object = [found firstObject];
            [m_framework setViewObject:object margin:16 minScale:10000];
            }
        }
    }

-(void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)aError
    {
    UIAlertView* alert = [[UIAlertView alloc]initWithTitle:@"Error" message:@"could not get your location" delegate:nil cancelButtonTitle:@"OK" otherButtonTitles: nil];
    [alert show];
    NSLog(@"Error: %@",aError.description);
    }

-(void)locationManager:(CLLocationManager *)aManager didUpdateLocations:(NSArray *)aLocations
    {
    CLLocation* new_location = [aLocations lastObject];
    if (!new_location)
        return;
    m_location = new_location;
    if (m_navigating)
        {
        CartoTypeNavigationData nav_data;
        std::memset(&nav_data,0,sizeof(nav_data));
        nav_data.validity = KTimeValid;
        nav_data.time = m_location.timestamp.timeIntervalSinceReferenceDate;
        if (m_location.horizontalAccuracy >= 0 && m_location.horizontalAccuracy <= 100)
            {
            nav_data.validity |= KPositionValid;
            nav_data.latitude = m_location.coordinate.latitude;
            nav_data.longitude = m_location.coordinate.longitude;
            }
        if (m_location.course >= 0)
            {
            nav_data.validity |= KCourseValid;
            nav_data.course = m_location.course;
            }
        if (m_location.speed >= 0)
            {
            nav_data.validity |= KSpeedValid;
            nav_data.speed = m_location.speed * 3.6; // convert from metres per second to kilometres per hour
            }
        if (m_location.verticalAccuracy >= 0 && m_location.verticalAccuracy <= 100)
            {
            nav_data.validity |= KHeightValid;
            nav_data.height = m_location.altitude;
            }
        
        [m_framework navigate:&nav_data];
        }
    }

@end
