//
// Copyright (C) 2010 Andras Varga
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
// - Gabriel Maciá Fernández (gmacia@ugr.es)
// - Leovigildo Sánchez Casado (sancale@ugr.es)
// - Rafael A. Rodríguez Gómez (rodgom@ugr.es)
// - Roberto Magán Carrión (rmagan@ugr.es)
// - Pedro García Teodoro (pgteodor@ugr.es)
// - José Camacho Páez (josecamacho@ugr.es)
// - Jesús E. Díaz Verdejo (jedv@ugr.es)
// 

//
// @titlepage
//
// <center><h1>NETwork Attacks Framework for OMNeT++</h1></center>
// <center>release 0.0.1</center>

//<b> NETwork Attacks (NETA)</b> is a framework devised to simulate attacks in heterogeneous networks using OMNeT++. 
//There exists a huge amount of research in network security and, with this framework, we pretend to give an 
//useful tool to researcher that are working on it. This tool could make easy to demonstrate the eﬀectiveness 
//of defence (detection or response) techniques against manet attacks or to compare the capabilities of different 
//defence techniques among many other applications. NETA is a <a href="http://inet.omnetpp.org/" target="_blank">INET 2.1.0</a> based framework. INET provides realistic 
//implementations of structures and protocols on each protocol layers, especially in the higher ones: network, 
//transport and application layers. For example, INET develops Mobile Ad-hoc NETworks (MANETs) routing protocols as AODV, DSR, OLSR, etc., 
//and several TCP implementations as well as several applications. Thanks to the versatile and extensible schema of NETA, 
//a huge amount attacks could be implemented in each protocol layer.
//
//NETA provides a useful tool for the research community for netowrk security threats concerns. NETA tries 
//to be a reference base tool for the network attacks simulation and implementation. 
//Is delivered under GPL license for free use of the research community, thus growing with the contributions 
//of this community.

// NETA makes the attack use and design easier!

//
// See the <tt><a href="whatsnew.html">WHATSNEW</a></tt> file in the distribution 
// for the list of recent changes.
//
// Introductory material:
//  - <a href="ma-architecture.html">NETA Framework architecture</a>
//  - <a href="../resources/frameworkDescription.pdf">NETA Framework User Manual (pdf)</a>
//
// Related documentation:
//   - <a href="../doxy/index.html" target="_top">C++ source documentation (Doxygen)</a>
//
// Installation procedure:
//	 <ul><li><a href="install.html">NETA installation guide</a></li></ul>
//
// <a href="history.html">History and contributors</a>.
//
// @page history.html, History
//
//   In mid of 2012, the NETA Framework concept begin to be a real purpose. 
//   It is built by the <a href="http://nesg.ugr.es" target="_blank">NESG</a> (Network Engineering & Security Group) to provide to the research
//   community a reference and useful tool for simulation and development network attacks.
//   
//   Currently, in mid of 2013, and architecture and several attacks examples has been created, corroborating the 
//   feasibility of this framework as tool to create and to use network attacks in whatever protocol layers and networks.
//
//	Regards to the contributors that have made it possible, currently are:
//
// <table>
//   <tr>  <td><a href="http://nesg.ugr.es/index.php?option=com_jresearch&view=member&task=show&id=9&Itemid=61" target="_blank">Gabriel Maci&aacute; Fern&aacute;ndez</a> (gmacia@ugr.es), project head and developer</td>    </tr>
//   <tr>  <td><a href="http://nesg.ugr.es/index.php?option=com_jresearch&view=member&task=show&id=7&Itemid=61" target="_blank">Leovigildo S&aacute;nchez Casado</a>  (sancale@ugr.es), developer.</td>     </tr>
//   <tr>  <td><a href="http://nesg.ugr.es/index.php?option=com_jresearch&view=member&task=show&id=8&Itemid=61" target="_blank">Rafael Rodr&iacute;guez G&oacute;mez</a>  (rodgom@ugr.es), developer.</td>   </tr>
//	 <tr>  <td><a href="http://nesg.ugr.es/index.php?option=com_jresearch&view=member&task=show&id=2&Itemid=61" target="_blank">Roberto Mag&aacute;n Carri&oacute;n</a>  (rmagan@ugr.es), developer.</td>   </tr>
//   <tr>  <td><a href="http://nesg.ugr.es/index.php?option=com_jresearch&view=member&task=show&id=6&Itemid=61" target="_blank">Pedro Garc&iacute;a Teodoro</a>  (pgteodor@ugr.es), developer.</td>   </tr>
//   <tr>  <td><a href="http://nesg.ugr.es/index.php?option=com_jresearch&view=member&task=show&id=10&Itemid=61" target="_blank">Jos&eacute; Camacho P&aacute;ez</a>  (josecamacho@ugr.es), developer.</td>   </tr>
//   <tr>  <td><a href="http://nesg.ugr.es/index.php?option=com_jresearch&view=member&task=show&id=4&Itemid=61" target="_blank">Jes&uacute;s E. D&iacute;az Verdejo</a>  (jedv@ugr.es), developer.</td>   </tr>
// </table>
//



package inet.neta;
