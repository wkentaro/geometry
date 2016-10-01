/*
 * Copyright (c) 2016, Kentaro Wada.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Kentaro Wada. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string>

#include <tf/transform_listener.h>
#include <ros/ros.h>

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "tf_velocity");

  // allow 2 or 3 command line arguments
  if (argc < 3 || argc > 4)
  {
    printf("Usage: tf_velocity source_frame target_frame [echo_rate]\n\n");
    printf("This will echo the transform from the coordinate frame of the source_frame\n");
    printf("to the coordinate frame of the target_frame. \n");
    printf("Note: This is the transform to get data from target_frame into the source_frame.\n");
    printf("Default echo rate is 1 if echo_rate is not given.\n");
    return -1;
  }

  ros::NodeHandle nh;
  ros::NodeHandle p_nh("~");

  ros::Publisher pub_twist = p_nh.advertise<geometry_msgs::TwistStamped>("output", 1);

  double rate_hz;
  if (argc == 4)
  {
    // read rate from command line
    rate_hz = atof(argv[3]);
  }
  else
  {
    // read rate parameter
    p_nh.param("rate", rate_hz, 1.0);
  }
  ros::Rate rate(rate_hz);

  tf::TransformListener tf_listener;

  std::string source_frameid = std::string(argv[1]);
  std::string target_frameid = std::string(argv[2]);

  // wait for up to one second for the first transforms to become avaiable
  tf_listener.waitForTransform(source_frameid, target_frameid, ros::Time(), ros::Duration(1.0));

  while (nh.ok())
  {
    ros::Time stamp = ros::Time::now();
    try
    {
      geometry_msgs::Twist twist;
      tf_listener.lookupTwist(/*tracking_frame=*/target_frameid,
                              /*observation_frame=*/source_frameid,
                              /*time=*/ros::Time(),
                              /*averaging_interval=*/ros::Duration(1.0),
                              /*twist=*/twist);

      geometry_msgs::TwistStamped twist_stamped;
      twist_stamped.header.frame_id = source_frameid;
      twist_stamped.header.stamp = stamp;
      twist_stamped.twist = twist;

      pub_twist.publish(twist_stamped);
    }
    catch(tf::TransformException& e)
    {
      ROS_WARN_STREAM("Failure at " << stamp << ". Exception thrown:" << e.what());
    }
    rate.sleep();
  }

  return 0;
};
