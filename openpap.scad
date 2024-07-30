T = 3;
motor_opening_d = 64;
corner_r = 8;
cube_w = motor_opening_d + 2*T - 2*corner_r;


module motor() {
  d1 = 59;
  d2 = 53;
  out_d = 22;
  h2 = 10.5;
  h1 = 44;
  out_h = 4.5;
  cylinder(h1, d1/2, d1/2);
  translate([0,0,h1]) cylinder(h2, d2/2, d2/2);
  translate([0,0,h1+h2]) cylinder(out_h, out_d/2, out_d/2);
}

module motor_mounts() {
  for (i=[0:2]) translate([i*7-10,15,0]) 
  difference() {
    cube([6, 15, 6]);
    translate([2,0,3]) cube([2, 15, 3]);
  }
  difference() {
    cylinder(5, 22/2+T, 22/2+1);
    cylinder(5, 22/2, 22/2);
  }
}

module display() {
  translate([-28/2,-14/2-5,-3]) {
    difference() {
      cube([28,28,3]);
      //for (i=[2, 28-2]) for (j=[2, 28-2]) translate([i,j,0]) cylinder(6,r=1);
    }
    translate([0,5,0]) cube([28,14,6]);
  }
}

module shell() {
  motor_top_inset = 16;
  motor_opening_h = 45 + motor_top_inset;
  h = motor_opening_h + T;
  filter_l = 54;
  filter_w = 35;
  filter_t = 5;

  translate([0,0,-h]) difference() {
    union() {
      translate([corner_r, corner_r, 0]) minkowski() {
        cube([cube_w,cube_w+30+T,h-1]);
        cylinder(r=corner_r,h=1);
      }
      // lid alignment
      for (i=[0,1]) for (j=[0,1]) translate([corner_r+i*cube_w,corner_r+j*cube_w,h]) cylinder(T/2,T,1, $fn=20);
    }
    translate([corner_r, 2*T+motor_opening_d+(corner_r-T), T]) minkowski() {
      cube([cube_w,20,h+10]);
      cylinder(r=corner_r-T,h=1);
    }
    translate([motor_opening_d/2+T,motor_opening_d/2+T,T]) cylinder(h=100, r=motor_opening_d/2);
        
    // motor notches
    for(i=[0,120,240]) translate([(motor_opening_d+2*T)/2,(motor_opening_d+2*T)/2,h]) 
    rotate([0,0,i-30]) translate([(motor_opening_d)/2,0,0]) cube([2,6,(15+motor_top_inset)*2], center=true);
    
    // filter
    translate([(motor_opening_d+2*T-filter_l)/2,1,0]) cube([filter_l, filter_t, filter_w]);
    translate([(motor_opening_d+2*T-6)/2,0,0]) cube([6, filter_t, 7]);
    for (i=[-3:3]) for (j=[0,1,2])
    translate([i*7.5+(motor_opening_d+2*T)/2,0,7+j*12]) rotate([-90,0,0]) cylinder(20, r=3);
    for (i=[-3:2]) for (j=[0,1])
    translate([(i+.5)*7.5+(motor_opening_d+2*T)/2,0,13+j*12]) rotate([-90,0,0]) cylinder(20, r=3);
    
    // wires
    for (i=[1:3]) translate([i*8+T+3,motor_opening_d-10,T+3]) rotate([-90,0,0]) cylinder(20, r=3);
      
    translate([(motor_opening_d+2*T)/2,cube_w+30+T+corner_r*2-1,(h+T)/2]) mirror([0,1,0]) rotate([90,90,0]) display();

    // button
    translate([(motor_opening_d+2*T)/2,cube_w+30+T+corner_r*2-18,(h+T)/2]) rotate([0,90,0]) cylinder(h=100, r=6);

    // screw holes
    for (i=[0,1]) for (j=[0,1]) translate([corner_r +i*cube_w,corner_r+j*cube_w,h-17]) cylinder(20,r=1, $fn=10);
    
  }
}

module lid() {
  difference() {
    union() {
      translate([corner_r, corner_r, 0]) minkowski() {
        cube([cube_w,cube_w+30+T,T-1]);
        cylinder(r=corner_r,h=1);
      }
      translate([corner_r+cube_w/2,corner_r+cube_w/2,0]) {
        cylinder(6+T, 22/2+2*T, 22/2+1+T);
        cylinder(6+T+20, 25/2, 23/2);
        translate([-3,0,0]) cube([6,cube_w,6]);
        translate([0,0,6]) rotate([-90,0,0]) cylinder(cube_w, r=3, $fn=20);
      }
    }
    // lid alignment
    for (i=[0,1]) for (j=[0,1]) translate([corner_r+i*cube_w,corner_r+j*cube_w,0]) cylinder(T/2,T,1, $fn=20);
    // screw holes
    for (i=[0,1]) for (j=[0,1]) translate([corner_r +i*cube_w,corner_r+j*cube_w,0]) cylinder(20,r=1, $fn=10);

    translate([corner_r+cube_w/2,corner_r+cube_w/2,0]) {
      cylinder(5, 22/2+T, 22/2+1);
      translate([0,0,-1]) cylinder(T/2+1, r=22/2+2*T);
      cylinder(100, r=19/2);
      translate([0,0,6]) rotate([-90,0,0]) cylinder(cube_w-6, r=1.6, $fn=20);
      translate([0,cube_w-7,0]) cylinder(7, r=1.6, $fn=20);
    }

  }
}

//motor();
//motor_mounts();
//shell();
//lid();
//display();
