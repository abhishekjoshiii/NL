/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Bus_script");

int main (int argc, char *argv[])
{
  
  uint32_t nCsma = 3;

  CommandLine cmd (__FILE__);
  cmd.Parse(argc,argv);
  
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
 
  // install protocol suites
  InternetStackHelper stack;
  
  // install prptocol stack on node 0 of p2p topology
  stack.Install(p2pNodes.Get(0));
  
  //install protocol stack on nodes in bus topology
  stack.Install(csmaNodes);
  
  // Configure and assign ip addresses to the interfaces of p2p nodes
  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0","255.0.0.0");
  
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
  
  // Configure and assign ip addresses to the interfaces of csma nodes of bus topology
 
 address.SetBase("20.0.0.0","255.0.0.0");
 
 Ipv4InterfaceContainer csmaInterfaces;
 csmaInterfaces = address.Assign(csmaDevices);
 
 // configure and install server application on last csma node of bus topology
  UdpEchoServerHelper echoServer (9);
  
  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (3));
 
 serverApps.Start (Seconds (1.0));
 serverApps.Stop (Seconds (10.0));
 
  // configure and install client application on node 0 of p2p topology
  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (3), 9);
 
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
 
  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
 
 // Enable routing between two networks 10.0.0.0 and 20.0.0.0
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
 // capture packets
  pointToPoint.EnablePcapAll ("second");
  csma.EnablePcap ("second", csmaDevices.Get (1), true);

  // animate bus topology
  AnimationInterface anim("bus.xml");
  
  // set positions of nodes in bus topology
  anim.SetConstantPosition(p2pNodes.Get(0),10.0,15.0);
  anim.SetConstantPosition(csmaNodes.Get(0),30.0,15.0);
  anim.SetConstantPosition(csmaNodes.Get(1),40.0,15.0);
  anim.SetConstantPosition(csmaNodes.Get(2),50.0,15.0);
  anim.SetConstantPosition(csmaNodes.Get(3),60.0,15.0);
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
  
  
  
  
  
  
  
  












