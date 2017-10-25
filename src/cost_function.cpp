#include "cost_function.h"
#include <iostream>
#include <iostream>
#include <math.h>
#include "vehicle.h"

const double MS_TO_MPH = 2.23694;

const double MPH_TO_MS = 0.44704;

CostFunction::CostFunction(Vehicle *v, vector<vector<double>> s){
  vehicle = v;
  sensor_fusion = s;
}

double CostFunction::Compute(){
  
  //compute cost
  double cost = 0;
  cost += ChangeLane();
  //cost += DistanceGoalLane();
  cost += Inefficiency();
  cost += Collision();
  cost += Buffer();
  cost += Target();

  std::cout << "COST " << cost << "\n";
  return cost;
}

double CostFunction::Target(){
  double cost =0;
  if(vehicle->collider.target_speed==0){
    return 0;
  }
  int end_lane = vehicle->trajectory.lane_end;
  int start_lane = vehicle->trajectory.lane_start;
  double diff = (vehicle->collider.target_speed - vehicle->speed)/vehicle->collider.target_speed;
  cost = pow(diff,2) * EFFICIENCY;    
  std::cout << "Target " << cost << " " << vehicle->collider.target_speed  << "\n";
  return cost;
}

double CostFunction::ChangeLane(){
  //Compute cost to change lane, penalizes lane Away fron the leftiest lane (fastest).
  int end_lane = vehicle->trajectory.lane_end;
  int start_lane = vehicle->trajectory.lane_start;
  double cost = 0;
  if(start_lane != end_lane){
    cost += 2*COMFORT;
  }
  /*if(end_lane !=0)
    cost += COMFORT;*/
  std::cout << "Change Lane " << cost << "\n";
  return cost;
}

double CostFunction::Inefficiency(){
  //Always, the best efficiency is when the speed is closest to the limit
  double cost = 0;
  double diff = (vehicle->update.target_v - vehicle->speed)/vehicle->update.target_v;
  cost = pow(diff,2) * EFFICIENCY;
  std::cout << "Inefficiency " << cost << "\n";
  return cost;
}

double CostFunction::Collision(){  
  double cost = 0;
  if(vehicle->collider.collision){   
    std::cout <<  "Compute Collision cost " <<  vehicle->collider.distance << " " << vehicle->collider.target_speed << " " <<  vehicle->speed  << "\n";
    double distance =  vehicle->collider.distance;
    //distance divided by the relative speed    
    double time_to_collide = vehicle->collider.distance/(abs( vehicle->collider.target_speed)*MPH_TO_MS);
    //distance = pow(distance, 2);
    //cost += exp(-distance/30)*COLLISION;
    
    //changing lane
    if((vehicle->trajectory.lane_end != vehicle->trajectory.lane_start)
       //car is in front
       && ((vehicle->collider.distance > 1 && (vehicle->speed-2) <= vehicle->collider.target_speed)
       //car is in the back
	   || (vehicle->collider.distance < -1 && (vehicle->speed+2) >= vehicle->collider.target_speed))){
      if(abs(vehicle->collider.distance) > 5) {
	cost = exp(-pow(time_to_collide,2))*COLLISION;
      } else if(vehicle->trajectory.lane_end!=vehicle->update.lane){
	std::cout << "@@@@@\n@@@@@@@@@@\n@@@@@@@@@@\n@@@@@@@@@@@";
	//PLCL or PLCR	
	cost = exp(-pow(time_to_collide,2))*COLLISION/1000;
	//adjuts speed       
      } else {
	cost = exp(-pow(time_to_collide,2))*COLLISION*100;
      }
      
    } else {    
      cost = exp(-pow(time_to_collide,2))*COLLISION;
    }
  }
  std::cout << "Collision " << cost << "\n";
  return cost;
}


double CostFunction::Buffer(){
  double cost = 0;

  if(vehicle->collider.closest_approach == 10000
     || vehicle->collider.closest_approach <= 0){
    return 0;
  }

  std::cout << "BUFFER " << vehicle->collider.closest_approach << "\n";
  
  if(vehicle->collider.closest_approach <= 3){
    cost += 10 * DANGER;    
  } else {
    double time_steps = vehicle->collider.closest_approach/vehicle->collider.target_speed;
    if(time_steps > DESIRED_BUFFER){
      return 0;
    }

    double multiplier = 1.0 - pow((time_steps / DESIRED_BUFFER),2);
    cost = multiplier * DANGER;
    //Ideal buffer is above 20 meters
    //cost += exp(-vehicle->collider.closest_approach/20)*DANGER;
  }
  std::cout << "Buffer " << cost << "\n";
  return cost;
}
