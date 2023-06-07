/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Star");

int main (int argc, char *argv[])
{
   // setting the default values
   Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (137));

  
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("14kb/s"));
 
  // Specify number of spoke nodes
  uint32_t nSpokes = 8;
  
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  //configuring point to point net devices and channel between hub and spoke nodes
  
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));  
    
  PointToPointStarHelper star (nSpokes, pointToPoint); 
  
  
  // install protocol stacks on spoke nodes and hub
  InternetStackHelper internet;
  star.InstallStack (internet);
  
  // Assigning the ip addresses to spoke nodes and hub
  star.AssignIpv4Addresses (Ipv4AddressHelper ("10.0.0.0", "255.0.0.0"));
  
  // to get the ip address of interface 0 of hub
  NS_LOG_INFO("Address of Hub: " << star.GetHubIpv4Address(0));
  
  // to get IP addresses of Spoke nodes
  for (uint32_t j = 0; j < star.SpokeCount (); ++j)
    {
       NS_LOG_INFO("Address of SpokeNode : " << j);
       NS_LOG_INFO("Address: " << star.GetSpokeIpv4Address(j));
  
    }
    
  // configure the packet sink Application on Hub
  uint16_t port = 50000;  // specifying port no of hub  
  
  Address hubLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", hubLocalAddress);
  
  //install Packet Sink Application on Hub
  ApplicationContainer hubApp = packetSinkHelper.Install (star.GetHub ());
  
  hubApp.Start (Seconds (1.0));
  hubApp.Stop (Seconds (10.0));
  
  // configure On Off Application
  OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
  
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  
  
  // install OnOffApplication on spoke nodes
  ApplicationContainer spokeApps;
  
  for (uint32_t i = 0; i < star.SpokeCount (); ++i)
    {
      AddressValue remoteAddress (InetSocketAddress (star.GetHubIpv4Address (i), port));
      
      NS_LOG_INFO("remote: " << star.GetHubIpv4Address(i));
      
      onOffHelper.SetAttribute ("Remote", remoteAddress);
      spokeApps.Add (onOffHelper.Install (star.GetSpokeNode (i)));
      NS_LOG_INFO("remote: " << star.GetSpokeNode(i));
    }
  
  
  spokeApps.Start (Seconds (1.0));
  spokeApps.Stop (Seconds (10.0));
  
  
  // Turn on global static routing so we can actually be routed across the star.
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();  
  
  pointToPoint.EnablePcapAll ("star");
  
  // Animating star topology
  AnimationInterface anim ("hus_star.xml");
  star.BoundingBox (1, 1, 100, 100);
  
  
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
   
   
   
   
   
   
   
    
    
    
    
    
    
    
    
