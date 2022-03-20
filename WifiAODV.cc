#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"

#include "ns3/aodv-helper.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Mob");

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  NodeContainer c;
  c.Create (20); //20 wireless nodes
  
  WifiHelper wifi;
wifi.SetStandard (WIFI_PHY_STANDARD_80211b); //many standards  - 80211a,b,n (2.4 and 5GHz),ac,ax,p(VANETs, and WAVEs - wireless access in vehicular environments) supported
  
  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac"); //AdhocWifiMac - used for fanet, StaWifiMac - similar to adhoc but used for gcs, ApWifiMac - needs router
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate54Mbps")); //because 80211b has 54Mbps speed
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default (); //Yans - yet another network simulator
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  NetDeviceContainer cDevices = wifi.Install (wifiPhy, mac, c); //mac - wifimac helper, wifiPhy - physical wifi helper, c - number of nodes (20)
 //
  NS_LOG_INFO ("Enabling AODV routing on all backbone nodes");
  AodvHelper aodv;
  //AODV Protocol is being used in FANETs
  InternetStackHelper internet;
  internet.SetRoutingHelper (aodv); //has effect on the next Install ()
  internet.Install (c);
  
  //
  //Assign IPv4 addresses to the device drivers (actually associated IPv4 interfaces) we jus created
  //
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("192.168.0.0", "255.255.255.0");
  Ipv4InterfaceContainer cInterfaces;
  cInterfaces = ipAddrs.Assign (cDevices);
//Mobility Model - 2D Mobility Model
MobilityHelper mobility;

mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (0.0), "MinY", DoubleValue (0.0), "DeltaX", DoubleValue (5.0), "DeltaY", DoubleValue (10.0), "GridWidth", UintegerValue (3), "LayoutType", StringValue ("RowFirst"));

mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (-100, 100, -100, 100)));
mobility.Install (c);
 UdpEchoServerHelper echoServer (9);
   
  ApplicationContainer serverApps = echoServer.Install (c.Get(0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  
 UdpEchoClientHelper echoClient (cInterfaces.GetAddress(0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps = echoClient.Install (c.Get(1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  
wifiPhy.EnablePcapAll ("Fanet_review2"); //pcap wireshark packet capture file
//Network Animation using Netanim
AnimationInterface anim("Review2.xml");
//Ascii trace metrics can be processed using Tracemetrics software
AsciiTraceHelper ascii;
wifiPhy.EnableAsciiAll(ascii.CreateFileStream("Review2.tr"));
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
