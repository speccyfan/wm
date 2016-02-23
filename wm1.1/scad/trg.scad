
$fn=50;

module trg(){
hull(){
translate([-1.5,-4,40])cube([3,8,1]);
translate([-1.5,-40,0])cube([3,80,1]);
}
}

one();
rotate([0,0,90])two();

module one(){

difference(){
	union(){
	cylinder(r=4,h=41);
	trg();
	}
	cylinder(r=2.6,h=41);

cube([12,12,40.5],center=true);
}

}



module two(){

difference(){
	union(){
	cylinder(r=4,h=41);
	trg();
	cylinder(d=11,h=3);
	}
	cylinder(r=2.6,h=41);

translate([0,0,39.5])cube([12,12,40.5],center=true);
}


}