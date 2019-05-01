//
//  ViewController.swift
//  CartoTypeSwiftDemo
//
//  Copyright © 2016-2019 CartoType Ltd. All rights reserved.
//

import UIKit
import GLKit
import CoreLocation

class ViewController: CartoTypeViewController, UISearchBarDelegate, CLLocationManagerDelegate
    {
    var m_framework: CartoTypeFramework!
    let m_ui_scale: Double = Double(UIScreen.main.scale)
    var m_route_start_in_degrees = CartoTypePoint(x:0, y:0)
    var m_route_end_in_degrees = CartoTypePoint(x:0, y:0)
    var m_last_point_pressed_in_degrees = CartoTypePoint(x:0, y:0)
    var m_search_bar: UISearchBar!
    var m_navigate_button: UIButton!
    var m_pushpin_id: UInt64 = 0
    var m_navigating: Bool = false
    var m_location_manager: CLLocationManager!
    var m_location: CLLocation!
    
    required init?(coder aDecoder: NSCoder)
        {
        super.init(coder: aDecoder)
        }
    
    override init(nibName nibNameOrNil: String?, bundle nibBundleOrNil: Bundle?)
        {
        super.init(nibName: nibNameOrNil, bundle: nibBundleOrNil)
        }

    init(aFrameWork: CartoTypeFramework!, aBounds: CGRect)
        {
        super.init(_:aFrameWork, bounds:aBounds)

        m_framework = aFrameWork
        self.becomeFirstResponder()
        
        // Create a search bar.
        m_search_bar = UISearchBar.init()
        m_search_bar.delegate = self
        m_search_bar.frame = CGRect(x:0,y:0,width:300,height:40)
        m_search_bar.layer.position = CGPoint(x: view.bounds.size.width / 2,y:40)
        
        // Show cancel button.
        m_search_bar.showsCancelButton = true
        
        // Set placeholder
        m_search_bar.placeholder = "place name"
        
        // Add the search bar to the view.
        view.addSubview(m_search_bar!)
            
        // Create a button to start and stop navigating.
        m_navigate_button = UIButton.init()
        m_navigate_button.frame = CGRect(x: 0, y: 0, width: 50, height: 25)
        m_navigate_button.layer.position = CGPoint(x:view.bounds.size.width / 2,y:view.bounds.size.height - 30)
        m_navigate_button.setTitleColor(UIColor.red, for: UIControlState.normal)
        m_navigate_button.backgroundColor = UIColor.white
        m_navigate_button.setTitle("Start", for: UIControlState.normal)
        m_navigate_button.addTarget(self, action: #selector(startOrEndNavigation), for: UIControlEvents.touchUpInside)
        m_navigate_button.isHidden = true
        view.addSubview(m_navigate_button!)

        // Set up location services.
        m_location_manager = CLLocationManager.init()
        m_location_manager.delegate = self
        m_location_manager.desiredAccuracy = kCLLocationAccuracyBest
        
        if (CLLocationManager.authorizationStatus() != CLAuthorizationStatus.authorizedAlways &&
            CLLocationManager.authorizationStatus() != CLAuthorizationStatus.authorizedWhenInUse)
            {
            m_location_manager.requestWhenInUseAuthorization()
            }
        
        // Set the vehicle location to a quarter of the way up the display.
        m_framework.setVehiclePosOffsetX(0, andY: 0.25)
        
        // Display turn instructions.
        m_framework.setTurnInstructions(false, abbreviate: true, width: 1.5, widthUnit: "in", position: NoticePositionTopLeft, textSize: 12, textSizeUnit: "pt");
        }
    
    override func viewWillTransition(to size: CGSize, with coordinator: UIViewControllerTransitionCoordinator)
        {
        super.viewWillTransition(to: size, with: coordinator)
        m_navigate_button.layer.position = CGPoint(x:size.width / 2,y:size.height - 30)
        }
    
    override func didReceiveMemoryWarning()
        {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
        }
    
    override func onTap(_ aPointInDegrees: CartoTypePoint)
        {
        }
    
    override func onLongPress(_ aPointInDegrees: CartoTypePoint)
        {
        m_last_point_pressed_in_degrees = aPointInDegrees;
        var p = m_last_point_pressed_in_degrees;
        m_framework.convert(&p, from: DegreeCoordType, to: DisplayCoordType)
        
        // Find nearby objects.
        let object_array = NSMutableArray.init()
        let pixel_mm = m_framework.getResolutionDpi() / 25.4
        m_framework.find(inDisplay: object_array, maxItems: 10, point: p, radius: ceil(2 * pixel_mm))
        
        // See if we have a pushpin.
        m_pushpin_id = 0
        for (cur_object) in object_array
            {
            if ((cur_object as! CartoTypeMapObject).getLayerName().isEqual("pushpin"))
                {
                m_pushpin_id = (cur_object as! CartoTypeMapObject).getId()
                break
                }
            }
            
        // Create the menu.
        let menu = UIMenuController.shared
        var pushpin_menu_item : UIMenuItem?
        if (m_pushpin_id != 0)
            {
            pushpin_menu_item = UIMenuItem.init(title: "Delete pin", action: #selector(deletePushPin))
            }
        else
            {
            pushpin_menu_item = UIMenuItem.init(title: "Insert pin", action: #selector(insertPushPin))
            }
        menu.menuItems = [
                         pushpin_menu_item!,
                         UIMenuItem.init(title: "Start here", action: #selector(ViewController.setRouteStart)),
                         UIMenuItem.init(title: "End here", action: #selector(ViewController.setRouteEnd)),
                         ]
        menu.setTargetRect(CGRect(x:p.x / m_ui_scale,y:p.y / m_ui_scale,width:1,height:1), in: view)
        menu.setMenuVisible(true, animated: true)
        }
    
    override var canBecomeFirstResponder: Bool { return true }
    
    func setRouteStart()
        {
        m_route_start_in_degrees = m_last_point_pressed_in_degrees
        if (m_route_end_in_degrees.x != 0 && m_route_end_in_degrees.y != 0)
            {
            m_navigate_button.isHidden = false
            m_framework.startNavigation(from: m_route_start_in_degrees, start: DegreeCoordType, to: m_route_end_in_degrees, end: DegreeCoordType)
            }
        }
    
    func setRouteEnd()
        {
        m_route_end_in_degrees = m_last_point_pressed_in_degrees
        if (m_route_end_in_degrees.x != 0 && m_route_end_in_degrees.y != 0)
            {
            m_framework.startNavigation(from: m_route_start_in_degrees, start: DegreeCoordType, to: m_route_end_in_degrees, end: DegreeCoordType)
            }
        m_navigate_button.isHidden = false
        }
    
    func insertPushPin()
        {
        let a : CartoTypeAddress = CartoTypeAddress.init()
        m_framework.getAddress(a, point: m_last_point_pressed_in_degrees, coordType: DegreeCoordType)
        let p : CartoTypeMapObjectParam = CartoTypeMapObjectParam.init(type: PointMapObjectType, andLayer: "pushpin", andCoordType: DegreeCoordType)
        p.appendX(m_last_point_pressed_in_degrees.x, andY: m_last_point_pressed_in_degrees.y)
        p.mapHandle = 0
        p.stringAttrib = a.toString(false)
        m_framework.insertMapObject(p)
        m_pushpin_id = p.objectId
        }
    
    func deletePushPin()
        {
        m_framework.deleteObjects(fromMap: 0, fromID: m_pushpin_id, toID: m_pushpin_id, withCondition: nil, andCount: nil)
        m_pushpin_id = 0
        }
    
    func startOrEndNavigation()
        {
        if (!m_navigating)
            {
            m_location_manager.startUpdatingLocation()
            UIApplication.shared.isIdleTimerDisabled = true
            
            // Try to get start of route if not known.
            if (m_framework.getRouteCount() == 0)
                {
                if (m_location != nil)
                    {
                    m_route_start_in_degrees.x = m_location.coordinate.longitude
                    m_route_start_in_degrees.y = m_location.coordinate.latitude
                    m_framework.startNavigation(from: m_route_start_in_degrees, start: DegreeCoordType, to: m_route_end_in_degrees, end: DegreeCoordType)
                    }
                }
            
            if (m_framework.getRouteCount() != 0)
                {
                m_navigate_button.setTitle("End", for: UIControlState.normal)
                m_search_bar.isHidden = true
                m_navigating = true
                }
            
            }
        else
            {
            m_navigate_button.setTitle("Start", for: UIControlState.normal)
            m_search_bar.isHidden = false
            m_navigating = false
            m_route_start_in_degrees.x = 0; m_route_start_in_degrees.y = 0
            m_route_end_in_degrees.x = 0; m_route_end_in_degrees.y = 0
            m_framework.deleteRoutes()
            m_navigate_button.isHidden = true
            }
        
        if (!m_navigating)
            {
            m_location_manager.stopUpdatingLocation()
            UIApplication.shared.isIdleTimerDisabled = false
            }
        }
        
    
    func searchBar(_ searchBar: UISearchBar, textDidChange aSearchText: String)
    {
    }

    func searchBarCancelButtonClicked(_ aSearchBar: UISearchBar)
    {
    view.endEditing(true)
    }

    func searchBarSearchButtonClicked(_ aSearchBar: UISearchBar)
    {
    view.endEditing(true)
    let text = aSearchBar.text
    if (text != nil)
        {
        let found : NSMutableArray = NSMutableArray.init()
        let param : CartoTypeFindParam = CartoTypeFindParam.init()
        param.text = text
        m_framework.find(found, with: param)
        if (found.count > 0)
            {
            let object : CartoTypeMapObject = found.firstObject as! CartoTypeMapObject
            m_framework.setViewObject(object, margin: 16, minScale: 10000)
            }
        }
    }
    
    func locationManager(_ manager: CLLocationManager, didFailWithError error: Error)
        {
        let alert_controller = UIAlertController.init(title: "Error", message: "could not get your location", preferredStyle: UIAlertControllerStyle.alert)
        present(alert_controller, animated: true, completion: nil)
        }
    
    func locationManager(_ manager: CLLocationManager, didUpdateLocations aLocations: [CLLocation])
        {
        let new_location = aLocations.last
        if (new_location == nil)
            {
            return
            }
        m_location = new_location!
        if (m_navigating)
            {
            var nav_data = CartoTypeNavigationData.init(validity: KTimeValid, time: m_location.timestamp.timeIntervalSinceReferenceDate, longitude: 0, latitude: 0, speed: 0, course: 0, height: 0)
            if (m_location.horizontalAccuracy > 0 && m_location.horizontalAccuracy <= 100)
                {
                nav_data.validity |= KPositionValid
                nav_data.latitude = m_location.coordinate.latitude
                nav_data.longitude = m_location.coordinate.longitude
                }
            if (m_location.course >= 0)
                {
                nav_data.validity |= KCourseValid
                nav_data.course = m_location.course
                }
            if (m_location.speed >= 0)
                {
                nav_data.validity |= KSpeedValid
                nav_data.speed = m_location.speed * 3.6 // convert from metres per second to kilometres per hour
                }
            if (m_location.verticalAccuracy >= 0 && m_location.verticalAccuracy <= 100)
                {
                nav_data.validity |= KHeightValid
                nav_data.height = m_location.altitude
                }
            m_framework.navigate(&nav_data)
            }
        }
    
    }
