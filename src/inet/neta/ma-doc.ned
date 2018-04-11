//
// Copyright (C) 2003 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
// 

//
// Copyright (C) 2013 and modified by NESG (Network Engineering and Security Group), http://nesg.ugr.es,
// - Gabriel MaciÃ¡ FernÃ¡ndez (gmacia@ugr.es)
// - Leovigildo SÃ¡nchez Casado (sancale@ugr.es)
// - Rafael A. RodrÃ­guez GÃ³mez (rodgom@ugr.es)
// - Roberto MagÃ¡n CarriÃ³n (rmagan@ugr.es)
// - Pedro GarcÃ­a Teodoro (pgteodor@ugr.es)
// - JosÃ© Camacho PÃ¡ez (josecamacho@ugr.es)
// - JesÃºs E. DÃ­az Verdejo (jedv@ugr.es)
//

// @page whatsnew.html, What's New
//
// <pre>
// <nohtml>
// @include ../WHATSNEW
// </nohtml>
// </pre>
//
// @page install.html, NETA installation guide
//
//The MA Framework can be compiled on any platform supported by OMNeT++.
//
//<h3>Prerequisites</h3>
//
//You should have a working OMNeT++ installation, version 4.2.1 or later.
//
//<h3>Installation steps</h3>
//<ol>
//<li> Install OMNeT++. Download the OMNeT++ source code from <a href="http://omnetpp.org">OMNET web site</a>.
//Copy the OMNeT++ archive to the directory where you want to install it and 
//extract it. Then enter the following commands:
//</li>
//
//	<ul><li>$ ./configure</li>
//	<li>$ make</li></ul>
//
//More details about the installation process can be found in 
//<a href="http://omnetpp.org/doc/omnetpp/InstallGuide.pdf">OMNET install guide</a>.
//
//<li> Import INET 2.1.0. Download INET 2.1.0 
//from <a href="http://omnetpp.org/download/contrib/models/inet-2.1.0-src.tgz">INET web site</a>. 
//Import it as a new project into your workspace (File > Import > General > Existing project 
//into Workspace) and select the option 'Select archive ï¬�le'.</li>
//
//<li> Copy and override the file ManetRoutingBase.cc from NETA framework located 
//into 'resources/patch/INET_21' folder to 
//'/inet/src/networklayer/manetrouting/base/' into the INET project previously 
//imported. This file fix the bug <a href="http://dev.omnetpp.org/bugs/view.php?id=632">632</a>.</li>
//
//<li>Build INET project (right-click on the project and select 'Build project').</li>
//
//<li> Import NETwork Attacks framework. The last version of the framework
//can be found in <a href="http://nesg.ugr.es/index.php/en/results">NESG NETA project</a>. Import it proceeding in similar way of the 
//process followed with INET in step 2.</li>
//
//<li> Make you sure that NETA project has INET project as project reference 
//(Project > Properties > Project References). Also, the 'MANET Routing' project 
//feature must be selected (Project > Properties > OMNET++ > Project Features)</li>
//
//<li>Build NETA project (right-click on the project and select 'Build project').</li>
//
//<li> Please check if the NETA framework is working correctly by running any 
//simple simulation escenario located under 'simulations' folder.</li>
//
//<li> And finally, enjoy it!</li>
//
// @page ma-architecture.html, NETA framework architecture
//
// 	The NETA framework uses the same idea as OMNeT++, i.e., modules that communicate by message passing.
//
//	The general idea of the present framework is to develop new nodes which can strike attacks.
//	In order to do this, the implemented attacks are managed in what we called attack controllers.
//	These controllers affect to one or more modules of INET framework through control
//	messages sending parameters of the developed attacks. Therefore, these modules should be
//	conveniently modiï¬�ed to obey the orders of the control messages. These modiï¬�ed modules
//	are named hacked modules. The creation of an attacker can be summarized as: <i> (i)</i>  add to
//	the associated <code>.ned</code>  ï¬�le the controllers related with the attacks to be executed, <i> (ii)</i> create the
//	associate control messages and, <i> (iii)</i> substitute the modules these attack controllers need to use
//	by the correspondent hacked modules.
//
//	In Fig. 1 we can see the comparison between a normal node and an attacker node. The
//	normal node is composed of simple and compound modules which communicate between them.
//	The attacker node is composed of the same number of modules and its communications and the
//	attack controller modules. However, some modules have been substituted by the correspondent
//	hacked modules to allow the execution of the attacks trigger by the included attack controllers.
//
//	<table border="0">
//		<caption align="bottom" style="text-aling: center; font-size: 10px; font-weight: bold;"> Fig. 1:  Scheme comparison between an original node and its attacker in NETwork Attacks frame-work. </caption> 
//		<tr> <td> <img src="../resources/img/schemeComparison.png" width="75%" > </td> </tr> 
//  </table> 
//   
//
//	In Fig. 2 we present an example of the module that compose a normal node <code> (AdhocHost)</code>  and
//	its correspondent attacker node <code>(MA_AdhocHost)</code> .
//
//	<table border="0">
//		<caption align="bottom" style="text-aling: center; font-size: 10px; font-weight: bold;"> Fig. 2:  Real comparison between an original AdhocHost and MA AdhocHost.</caption> 
//		<tr> 
//			<td> <img src="../resources/img/adhoc.png" width="85%" > </td> 
//			<td> <img src="../resources/img/MAAdhoc.png" width="85%" > </td>
//		</tr> 
//		<tr > 
//			<td style="text-aling: center; font-size: 10px; "> (a)  Modules of AdhocHost node </td> 
//			<td style="text-aling: center; font-size: 10px; "> (b)  Modules of MA AdhocHost node</td>
//		</tr>
//  </table> 
//
//  <h3>Folder structure</h3> 
//
//	The folders of this framework has a speciï¬�c structure we describe in the following (we only
//	mention the speciï¬�c folders of this framework excluding the folders in common with OMNeT++):
//  
//
//	<ul>
//  	<li><b>src:</b> 
//       <ul>
//       	<li>attacks : In this folder are located the attack controllers and the control messages used by these
//								controllers to communicate with the associated hacked modules.
//			</li> 
//         	<li>hackedmodules : Here it can be found all the modiï¬�ed modules to be able of triggering the implemented
//									attacks by this release, i.e., the hacked modules.
//			</li>
//			<li>nodes : In this folder it can be found all the new hosts that are a slight modiï¬�cation of
//							the existent ones in INET framework. They have inside of them the
//							correspondent hacked modules.
//			</li>
//       </ul>
//		</li>
//  	<li><b>simulations:</b>
//			Here the simulations to demonstrate the use of the implemented attacks by this framework
//			are present.
//		</li>
//	</ul>
//
// For more information about the NETA framework architecture, please read the <a href="../resources/frameworkDescription.pdf">NETA Framework manual</a>.
//




package inet.neta;
