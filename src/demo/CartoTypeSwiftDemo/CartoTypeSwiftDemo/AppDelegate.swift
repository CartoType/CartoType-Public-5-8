//
//  AppDelegate.swift
//  CartoTypeSwiftDemo
//
//  Copyright © 2016-2019 CartoType Ltd. All rights reserved.
//

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    var window: UIWindow?


    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplicationLaunchOptionsKey: Any]?) -> Bool
    {
        // Create the CartoType framework
        let bounds = UIScreen.main.bounds;
        self.window = UIWindow.init(frame: bounds)
        let scale = UIScreen.main.scale
        let width = bounds.width * scale
        let height = bounds.height * scale
        let param = CartoTypeFrameworkParam()
        param.mapFileName = "santa-cruz"
        param.styleSheetFileName = "standard"
        param.fontFileName = "DejaVuSans"
        param.viewWidth = Int32(width)
        param.viewHeight = Int32(height)
        let framework = CartoTypeFramework.init(param: param)
        
        // Uncomment the following line to create walking routes, not driving routes.
        // framework?.setMainProfileType(WalkingProfile)
        
        // Add a scale bar.
        framework?.setScaleBar(false,width: 1.75,unit: "in",position: NoticePositionBottomLeft)

        // Create the view controller.
        let view_controller = ViewController.init(aFrameWork: framework, aBounds:bounds)
        self.window?.rootViewController = view_controller
        self.window?.backgroundColor = UIColor.white
        self.window?.makeKeyAndVisible()

        return true
    }

    func applicationWillResignActive(_ application: UIApplication) {
        // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    }

    func applicationDidEnterBackground(_ application: UIApplication) {
        // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
        // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
        // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
    }

    func applicationDidBecomeActive(_ application: UIApplication) {
        // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    }

    func applicationWillTerminate(_ application: UIApplication) {
        // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    }


}

