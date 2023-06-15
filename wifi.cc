/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Bus_script");

int 
main (int argc, char *argv[])
{
  
  uint32_t nCsma = 3;
  uint32_t nWifi = 3;

  CommandLine cmd (__FILE__);
  cmd.Parse(argc,argv);
  
  // no of wifi nodes should be less than 18
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  
  // set time resolution
  Time::SetResolution (Time::NS);

  // enable logging for client and server applications
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
  // Create point to point nodes in p2p topology
  NodeContainer p2pNodes;
  p2pNodes.Create(2);
  
  //Configure net devices and channel on point to point nodes
  PointToPointHelper pointToPoint;
  
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  // install net devices on point to point nodes
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  //creating bus topology
  NodeContainer csmaNodes;
  csmaNodes.Add(p2pNodes.Get(1));
  csmaNodes.Create(nCsma);
 
  // configuring net devices and channel on csma nodes in bus topology
  CsmaHelper csma;
  
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  
  // install net devices on csma nodes
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);
 
  // create wifi nodes
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  // add node n0 to wifi nodes
  NodeContainer wifiApNode = p2pNodes.Get (0); // wifi access point
 
  // configure wifi channel on wifi nodes
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default (); 
  
  // create and manage PHY objects for the yans model
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  
  //Every PHY created by a call to Install is associated to this channel. 
  //Create a channel based on the configuration parameters set previously. 
  phy.SetChannel (channel.Create ());
 
  // configure wifi net devices
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager"); // specify type of wifi remote station manager
  
  //create MAC layers for a wifi net device
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid"); //Create SSID from a given string. 
  
  //Specify the type of ns3::WifiMac to create and configure other attributes for wifi nodes
  mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));
 
  // Install configured wifi channel and wifi net devices on wifi nodes
  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);
  
  //Specify the type of ns3::WifiMac to create and configure other attributes for wifi access point N0
  mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
  
  // Install configured wifi channel and wifi net devices on wifi access point
  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);
 
 
  // assign positions and mobility models to nodes
  MobilityHelper mobility;
  
  
  // Specify the type of mobility model to use and configure other attributes for wifi nodes.
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
 
  //will create an instance of a matching mobility model for each wifi node. 
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  
  
  //Layout a collection of wifi nodes according to the current position allocator type.
  mobility.Install (wifiStaNodes);

  
  //will create an instance of a matching mobility model for access point node. 
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  
  //Layout a access point node according to the current position allocator type.
  mobility.Install (wifiApNode);
 
 
  // install protocol suites
  InternetStackHelper stack;
  
  // install protocol stack on csma nodes in bus topology
  stack.Install (csmaNodes);
  
  //install protocol stack on access point node 
  stack.Install (wifiApNode);
  
  // install protocol stack on wifi nodes
  stack.Install (wifiStaNodes);
  
  // Configure and assign ip addresses to the interfaces of p2p nodes
  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0","255.0.0.0");
  
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
  
  // Configure and assign ip addresses to the interfaces of csma nodes of bus topology
 
  address.SetBase("30.0.0.0","255.0.0.0");
 
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign(csmaDevices);
 
  // Configure and assign ip addresses to the wifi nodes and access point 
  address.SetBase ("20.0.0.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);
  
  
 // configure and install server application on last csma node of bus topology
  UdpEchoServerHelper echoServer (9);
  
  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
 
 serverApps.Start (Seconds (1.0));
 serverApps.Stop (Seconds (10.0));
 
  // configure and install client application on node 0 of p2p topology
  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
 
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
 
  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (2));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
 
 // Enable routing between two networks 10.0.0.0 and 20.0.0.0
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
 // capture packets
  //pointToPoint.EnablePcapAll ("second");
  //csma.EnablePcap ("second", csmaDevices.Get (1), true);

  Simulator::Stop (Seconds (10.0));
  
  AnimationInterface anim("wifi_example.xml");
  
  anim.SetConstantPosition(p2pNodes.Get(0),22.0,38.0);
  anim.SetConstantPosition(csmaNodes.Get(0),42.0,38.0);
  anim.SetConstantPosition(csmaNodes.Get(1),62.0,15.0);
  anim.SetConstantPosition(csmaNodes.Get(2),72.0,15.0);
  anim.SetConstantPosition(csmaNodes.Get(3),82.0,15.0);
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
  
  
  
  
  
  
  
  












