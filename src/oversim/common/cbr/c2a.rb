#!/usr/bin/ruby

#
# Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

# == Synopsis
#
# Coordinates to Areas (c2a): Calculates node bisecting borders for a given n-dim coordinate XML file and maps an ID-prefix on each area.
#
# Optional Parameters: see below
#
# == Usage
#
# c2a.rb [OPTION] ... nodefile.xml
#
# -h, --help: 
#   show this help
# -n x, --minnodes x: 
#   don't split area with x or less nodes (default: 1)
# -p x, --maxprefix x:
#   maximum prefix length in bits (default: 32)
# -d, --debug:
#   prints calculated borders on std out
# -X, --noxml:
#   disables writing into area xml file (only useful with --debug or --plot)
# -P, --plot:
#   writes into Gnuplot compatible plot files for figures (2D and 3D only)
#
# nodefile.xml: The XML file to be processed

require 'getoptlong'
#require 'xml/libxml_so'
#requires libxml-ruby
#require 'rdoc/usage'
require 'xml'

opts = GetoptLong.new(
    [ '--help', '-h', GetoptLong::NO_ARGUMENT ],
    [ '--debug', '-d', GetoptLong::NO_ARGUMENT ],    
    [ '--noxml', '-X', GetoptLong::NO_ARGUMENT ],
    [ '--plot', '-P', GetoptLong::NO_ARGUMENT ],
    [ '--minnodes', '-n', GetoptLong::REQUIRED_ARGUMENT ],
    [ '--maxprefix', '-p', GetoptLong::REQUIRED_ARGUMENT ],
    [ '--fieldsize', '-f', GetoptLong::REQUIRED_ARGUMENT ]
)

nodefile = nil
minnodes = 1
maxprefix = 80
debugoutput = false
plotoutput = false
xmloutput = true
fieldsize = 1000
opts.each do |opt, arg|
    case opt
        when '--help'
            RDoc::usage
        when '--debug'
            debugoutput = true
        when '--noxml'
            xmloutput = false
        when '--plot'
            plotoutput = true
        when '--minnodes'
            minnodes = arg.to_i
        when '--maxprefix'
            maxprefix = arg.to_i
        when '--fieldsize'
            fieldsize = arg.to_i
    end
end

unless ARGV.length == 1
    puts "Missing file argument (try --help)"
    exit 0
end
nodefile = ARGV.shift


########################### Constants ################################
MAXPREFIX = maxprefix
MINNODES = minnodes
MIN=-fieldsize
MAX=fieldsize
DEBUGOUTPUT = debugoutput
PLOTOUTPUT = plotoutput
XMLOUTPUT = xmloutput
########################### Treenode Class ############################
class Treenode
    @@id = 0
    def initialize(nodes, prefix, bottoms, tops, depth)
        @id = @@id
        @@id += 1
        @bottoms = bottoms
        @tops = tops
        @children = []
        @nodes = nodes.sort_by { |node| node[depth % DIMENSIONS] } if nodes != nil
        @prefix = prefix
        @depth = depth
        @thisdim = depth % DIMENSIONS
    end
    
    def addChild(nodes, prefix, bottoms, tops)
        @children << Treenode.new(nodes, prefix, bottoms, tops, @depth + 1)
    end

    def saveDataToPlotfile(file)
        if @children.length > 0
            @children.each do |child|
                child.saveDataToPlotfile(file)
            end
        else
          if DIMENSIONS == 2
            file.print "#{@bottoms[0]} #{@bottoms[1]}\n"
            file.print "#{@bottoms[0]} #{@tops[1]}\n"
            file.print "#{@tops[0]} #{@tops[1]}\n"
            file.print "#{@tops[0]} #{@bottoms[1]}\n"
            file.print "#{@bottoms[0]} #{@bottoms[1]}\n"
            file.print "\n"
          elsif DIMENSIONS == 3
            # x/y lo
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            file.print "#{@bottoms[0]} #{@tops[1]} #{@bottoms[2]}\n"
            file.print "#{@tops[0]} #{@tops[1]} #{@bottoms[2]}\n"
            file.print "#{@tops[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            # x/z lo 
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@tops[2]}\n"
            file.print "#{@tops[0]} #{@bottoms[1]} #{@tops[2]}\n"
            file.print "#{@tops[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            # y/z lo 
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@tops[2]}\n"
            file.print "#{@bottoms[0]} #{@tops[1]} #{@tops[2]}\n"
            file.print "#{@bottoms[0]} #{@tops[1]} #{@bottoms[2]}\n"
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            # x/y hi 
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@tops[2]}\n"
            file.print "#{@bottoms[0]} #{@tops[1]} #{@tops[2]}\n"
            file.print "#{@tops[0]} #{@tops[1]} #{@tops[2]}\n"
            file.print "#{@tops[0]} #{@bottoms[1]} #{@tops[2]}\n"
            file.print "#{@bottoms[0]} #{@bottoms[1]} #{@tops[2]}\n"
            # over to x/z
            file.print "#{@bottoms[0]} #{@tops[1]} #{@tops[2]}\n"
            # x/z hi 
            file.print "#{@bottoms[0]} #{@tops[1]} #{@bottoms[2]}\n"
            file.print "#{@bottoms[0]} #{@tops[1]} #{@tops[2]}\n"
            file.print "#{@tops[0]} #{@tops[1]} #{@tops[2]}\n"
            file.print "#{@tops[0]} #{@tops[1]} #{@bottoms[2]}\n"
            file.print "#{@bottoms[0]} #{@tops[1]} #{@bottoms[2]}\n"
            # over to y/z
            file.print "#{@tops[0]} #{@tops[1]} #{@bottoms[2]}\n"
            # y/z hi 
            file.print "#{@tops[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            file.print "#{@tops[0]} #{@bottoms[1]} #{@tops[2]}\n"
            file.print "#{@tops[0]} #{@tops[1]} #{@tops[2]}\n"
            file.print "#{@tops[0]} #{@tops[1]} #{@bottoms[2]}\n"
            file.print "#{@tops[0]} #{@bottoms[1]} #{@bottoms[2]}\n"
            # return to x/y/z bottom
            file.print "\n\n"
          end
        end
    end

    def saveRawCoordsToPlotfile(file)
        # don't descend into children, just top level of tree where all nodes are
        @nodes.each do |node|
            file.print "#{node[0]} #{node[1]}\n"              if DIMENSIONS == 2
            file.print "#{node[0]} #{node[1]} #{node[2]}\n"   if DIMENSIONS == 3
        end
    end

    def saveToXml(file)
        file.print "<!DOCTYPE arealist SYSTEM \"areas.dtd\">\n"
        file.print "<arealist dimensions=\"#{DIMENSIONS}\">\n"
        saveDataToXml(file)
        file.print "</arealist>\n"
    end

    def saveDataToXml(file)
        if @children.length > 0
            @children.each do |child|
                child.saveDataToXml(file)
            end
        else
            file.print "\t<area>\n"
            DIMENSIONS.times do |dim|
                file.print "\t\t<min dimension=\"#{dim}\">#{@bottoms[dim]}</min>\n"
            end
            DIMENSIONS.times do |dim|
                file.print "\t\t<max dimension=\"#{dim}\">#{@tops[dim]}</max>\n"
            end
            file.print "\t\t<prefix>#{@prefix}</prefix>\n"
            file.print "\t</area>\n"
        end
    end

    def printAllData
        if @children.length > 0
            @children.each do |child|
                child.printAllData
            end
        else
            numnodes = @nodes.length    unless @nodes == nil
            print "id: #{@id}"
            DIMENSIONS.times do |dim|
                print "bottom[#{dim}]: #{@bottoms[dim]}, "
            end
            DIMENSIONS.times do |dim|
                print "top[#{dim}]: #{@tops[dim]}, "
            end
            print "prefix: #{@prefix}, nodes left: #{numnodes}"
            print "\n"
        end
    end

    def splitByNodes(returnval)
        #returnval = 0: return lower half node set
        #returnval = 1: return upper half node set
        #returnval = 2: return split coordinate
        num = @nodes.length
        half = (num / 2).floor
        splitcoord = (@nodes[half-1][@thisdim] + @nodes[half][@thisdim]) / 2
        case returnval
            when 0
                return @nodes[0..half-1]
            when 1
                return @nodes[half..num-1]
            when 2
                return splitcoord
        end
    end

    def split()
        # split unless no more nodes left or maximum prefix length is reached
        unless @nodes == nil || @depth >= MAXPREFIX
            lnodes = splitByNodes(0)
            unodes = splitByNodes(1)
            splitcoord = splitByNodes(2);

            lnodes = nil    if lnodes.length <= MINNODES
            unodes = nil    if unodes.length <= MINNODES
        
            newbottoms = []
            @bottoms.each do |bottom|
                newbottoms << bottom
            end
            newbottoms[@thisdim] = splitcoord

            newtops = []
            @tops.each do |top|
                newtops << top
            end
            newtops[@thisdim] = splitcoord

            addChild(lnodes, @prefix+"0", @bottoms, newtops); 
            addChild(unodes, @prefix+"1", newbottoms, @tops);

            @children.each do |child|
                child.split
            end
        end
    end
end

######################## MAIN #################################
# loading the xml file
puts "Maximum prefix length wanted: #{MAXPREFIX}"
puts "Minimum number of nodes per section wanted: #{MINNODES}"
puts
puts "Loading XML file into memory... "
xmlfile = File.read(nodefile)
parser = XML::Parser.new
parser.string = xmlfile
xmldoc = parser.parse
puts "...done!"

# print some statistics and set basic constants...
nodesnum = xmldoc.find('//nodelist/node').length
#DIMENSIONS = xmlroot.attributes['dimensions'].to_i
DIMENSIONS = xmldoc.root['dimensions'].to_i
puts
puts "#{nodesnum} nodes found"
puts "Number of dimensions: #{DIMENSIONS}"
puts

# nodes: for each node in xml get isroot, ip and coords
startnodes = [] 
puts "Parsing XML data..."
xmldoc.find("//nodelist/node").each do |node|
    coords = []
    node.find('coord').each do |coord|
        coords << coord.content.to_f
    end
    startnodes << coords
    if startnodes.length % 10000 == 0
        puts "#{startnodes.length} of #{nodesnum} nodes processed..."
    end 
end
puts "...done!"

puts "Calculating areas..."
startbottoms = []
starttops = []
DIMENSIONS.times do |dim|
    startbottoms[dim] = MIN
    starttops[dim] = MAX
end
root = Treenode.new(startnodes, "", startbottoms, starttops, 0)
root.split
puts "...done!"

# For debugging (switched on/off by DEBUGOUTPUT in constants)
if DEBUGOUTPUT
    root.printAllData
end

if PLOTOUTPUT
    if (DIMENSIONS == 2 || DIMENSIONS == 3)
      puts "Writing coords and areas into Gnuplot compatible files..."
      borderfile = File.open("plot_areas_"+nodefile, "w");
      coordsfile = File.open("plot_coords_"+nodefile, "w");
      root.saveDataToPlotfile(borderfile)
      root.saveRawCoordsToPlotfile(coordsfile)
      puts "...done"
    else
      puts "Gnuplot files only for 2D and 3D - skipping..."
    end
end

# XML Output
if XMLOUTPUT
  outputname = "areas_"+nodefile
  outputxml = File.open(outputname, "w");
  puts "Writing calculated areas to " << outputname << "..."
  root.saveToXml(outputxml)
  puts "...done!"
end
