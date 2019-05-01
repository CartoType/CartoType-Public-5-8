/*
ViewController.h
 
A view controller for the TestGL demo program.
*/

#import <GLKit/GLKit.h>
#import <CoreLocation/CLLocationManager.h>
#import <CoreLocation/CLLocationManagerDelegate.h>
#import <CartoType/CartoType.h>


@interface ViewController : CartoTypeViewController <UIGestureRecognizerDelegate, UISearchBarDelegate, CLLocationManagerDelegate>

-(id)init:(CartoTypeFramework*)aFramework bounds:(CGRect)aBounds;

@end
