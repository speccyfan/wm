$fn=50;
include <nuts_and_bolts.scad>
module screwhole(screw, depth)
{
translate([0,0,-0.5]) cylinder(r=screw/2, h=depth+1);
translate([0,0,-0.01]) nutHole(size=screw);
}




cyls=5;


difference(){
union(){

cylinder(d=11,h=6);
translate([0,0,-2])cylinder(d=20.5,h=5);

for ( i = [0 : cyls-1] )
{
    rotate( i * 360 / cyls, [0, 0, 1])
    translate([0, 7, -2])
	 cylinder(h=5, r = 5.8);
}

}

translate([0,0,-2])screwhole(5,8);
}