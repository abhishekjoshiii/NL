/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DhcpExample");

int main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  // set time resolution
  Time::SetResolution (Time::NS);
  
  // Enable logging for applications
  LogComponentEnable ("DhcpServer", LOG_LEVEL_ALL);
  LogComponentEnable ("DhcpClient", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  
  //create nodes
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  NodeContainer router;
  nodes.Create (3);
  router.Create (2);
  
  //combine routers and nodes into single object
  NodeContainer net (nodes, router);

  //configuring the csma net devices and csma channel
  CsmaHelper csma;
  
  csma.SetChannelAttribute ("DataRate", StringValue ("5Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("2ms"));
  csma.SetDeviceAttribute ("Mtu", UintegerValue (1500));

  //install configured net devices and channel on nodes
 NetDeviceContainer devNet = csma.Install (net);
 
 // creating p2p topology
  NodeContainer p2pNodes;
  p2pNodes.Add (net.Get (4)); //add node R1 to p2p
  p2pNodes.Create (1); // create new node A
  
  // configure p2p net devices and channel on R1 and A
  
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
 
  // install configured p2p net devices and channel on p2pnodes
  
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);
  
  // install protocol stack
 
  InternetStackHelper tcpip;
  tcpip.Install (nodes); //install on N0 N1 and N2
  tcpip.Install (router); // install on R0 and R1
  tcpip.Install (p2pNodes.Get (1)); //install on node A
  
  // assign IP address to p2p interfaces
  Ipv4AddressHelper address;
  address.SetBase ("20.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
  
  // Assign fixed IP address to default router R1
  DhcpHelper dhcpHelper;

  Ipv4InterfaceContainer fixedNodes = dhcpHelper.InstallFixedAddress (devNet.Get (4), Ipv4Address ("10.0.0.17"), Ipv4Mask ("/8"));
 
 // enable forwarding of packets from R1
 fixedNodes.Get (0).first->SetAttribute ("IpForward", BooleanValue (true));
 
 // enable routing between 2 networks
 Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
 //configure and install dhcp server on R0
 ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer (devNet.Get (3), Ipv4Address ("10.0.0.12"),Ipv4Address ("10.0.0.0"), Ipv4Mask ("/8"),Ipv4Address ("10.0.0.10"), Ipv4Address ("10.0.0.15"),Ipv4Address ("10.0.0.17"));
 
 // configure and start and stop time of server
  dhcpServerApp.Start (Seconds (0.0));
  dhcpServerApp.Stop (Seconds (20.0));
  
  // configuring DHCP clients
  NetDeviceContainer dhcpClientNetDevs;
  dhcpClientNetDevs.Add (devNet.Get (0));
  dhcpClientNetDevs.Add (devNet.Get (1));
  dhcpClientNetDevs.Add (devNet.Get (2));
  
  //install DHCP clients on N0 N1 and N2
  ApplicationContainer dhcpClients = dhcpHelper.InstallDhcpClient (dhcpClientNetDevs);
  
  //configure start and stop time of dhcpclient
  dhcpClients.Start (Seconds (1.0));
  dhcpClients.Stop (Seconds (20.0));
  
 // configure and install UdpEchoServer App on node A
  UdpEchoServerHelper echoServer (9); //port no

  ApplicationContainer serverApps = echoServer.Install (p2pNodes.Get (1));
  
  //configure start and stop time of UdpEchoServer 
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (20.0));
 
 // configure UdpEchoClient app on node N1
  UdpEchoClientHelper echoClient (p2pInterfaces.GetAddress (1), 9);
  
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
 
 ApplicationContainer clientApps = echoClient.Install (nodes.Get (1));
 
 //configure start and stop time of UdpEchoClient
  clientApps.Start (Seconds (10.0));
  clientApps.Stop (Seconds (20.0));
 
 //configure stop time of simulator
  Simulator::Stop (Seconds (30.0));

  // capture packets on all nodes in bus topology
  csma.EnablePcapAll ("dhcp-csma");
  
  // capture packets on p2p nodes
  pointToPoint.EnablePcapAll ("dhcp-p2p");

  // create animation object
  AnimationInterface anim("dhcp.xml"); // specify the output filename

  // set the attributes of animation
  anim.SetConstantPosition(nodes.Get(0), 10, 10);
  anim.SetConstantPosition(nodes.Get(1), 30, 10);
  anim.SetConstantPosition(nodes.Get(2), 20, 30);
  anim.SetConstantPosition(router.Get(0), 40, 10);
  anim.SetConstantPosition(router.Get(1), 40, 30);
  anim.SetConstantPosition(p2pNodes.Get(1), 50, 20);


  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  return 0;
}
 
 
 
 



