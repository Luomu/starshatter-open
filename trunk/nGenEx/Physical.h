/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Physical.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract Physical Object
*/

#ifndef Physical_h
#define Physical_h

#include "Types.h"
#include "Geometry.h"
#include "Camera.h"

// +--------------------------------------------------------------------+

class Director;
class Graphic;
class Light;

// +--------------------------------------------------------------------+

class Physical
{
public:
    static const char* TYPENAME() { return "Physical"; }

    Physical();
    Physical(const char* n, int t=0);
    virtual ~Physical();

    int operator == (const Physical& p) const { return id == p.id; }

    // Integration Loop Control:
    static void       SetSubFrameLength(double seconds) { sub_frame = seconds; }
    static double     GetSubFrameLength()               { return sub_frame;    }

    // operations
    virtual void      ExecFrame(double seconds);
    virtual void      AeroFrame(double seconds);
    virtual void      ArcadeFrame(double seconds);

    virtual void      AngularFrame(double seconds);
    virtual void      LinearFrame(double seconds);

    virtual void      CalcFlightPath();

    virtual void      MoveTo(const Point& new_loc);
    virtual void      TranslateBy(const Point& ref);
    virtual void      ApplyForce(const Point& force);
    virtual void      ApplyTorque(const Point& torque);
    virtual void      SetThrust(double t);
    virtual void      SetTransX(double t);
    virtual void      SetTransY(double t);
    virtual void      SetTransZ(double t);
    virtual void      SetHeading(double r, double p, double y);
    virtual void      LookAt(const Point& dst);
    virtual void      ApplyRoll(double roll_acc);
    virtual void      ApplyPitch(double pitch_acc);
    virtual void      ApplyYaw(double yaw_acc);

    virtual int       CollidesWith(Physical& o);
    static  void      ElasticCollision(Physical& a, Physical& b);
    static  void      InelasticCollision(Physical& a, Physical& b);
    static  void      SemiElasticCollision(Physical& a, Physical& b);
    virtual void      InflictDamage(double damage, int type = 0);

    // accessors:
    int               Identity()  const { return id;         }
    int               Type()      const { return obj_type;   }
    const char*       Name()      const { return name;       }

    Point             Location()  const { return cam.Pos();  }
    Point             Heading()   const { return cam.vpn();  }
    Point             LiftLine()  const { return cam.vup();  }
    Point             BeamLine()  const { return cam.vrt();  }
    Point             Velocity()  const { return velocity + arcade_velocity; }
    Point             Acceleration()
    const { return accel;      }
    double            Thrust()    const { return thrust;     }
    double            TransX()    const { return trans_x;    }
    double            TransY()    const { return trans_y;    }
    double            TransZ()    const { return trans_z;    }
    double            Drag()      const { return drag;       }

    double            Roll()      const { return roll;       }
    double            Pitch()     const { return pitch;      }
    double            Yaw()       const { return yaw;        }
    Point             Rotation()  const { return Point(dp,dr,dy); }

    double            Alpha()     const { return alpha;      }

    double            FlightPathYawAngle()    const { return flight_path_yaw;     }
    double            FlightPathPitchAngle()  const { return flight_path_pitch;   }

    double            Radius()    const { return radius;     }
    double            Mass()      const { return mass;       }
    double            Integrity() const { return integrity;  }
    double            Life()      const { return life;       }

    double            Shake()     const { return shake;      }
    const Point&      Vibration() const { return vibration;  }

    const Camera&     Cam()       const { return cam;        }
    Graphic*          Rep()       const { return rep;        }
    Light*            LightSrc()  const { return light;      }

    Director*         GetDirector() const { return dir;      }

    // mutators:
    virtual void      SetAngularRates(double  r, double  p, double  y);
    virtual void      GetAngularRates(double& r, double& p, double& y);
    virtual void      SetAngularDrag(double  r, double  p, double  y);
    virtual void      GetAngularDrag(double& r, double& p, double& y);
    virtual void      GetAngularThrust(double& r, double& p, double& y);
    virtual void      SetVelocity(const Point& v) { velocity = v; }
    virtual void      SetAbsoluteOrientation(double roll, double pitch, double yaw);
    virtual void      CloneCam(const Camera& cam);
    virtual void      SetDrag(double d) { drag = (float) d; }

    virtual void      SetPrimary(const Point& loc, double mass);
    virtual void      SetGravity(double g);
    virtual void      SetBaseDensity(double d);

    virtual double    GetBaseDensity()  const { return Do; }
    virtual double    GetDensity()      const;

    enum { NAMELEN = 48 };

protected:
    static int        id_key;

    // identification:
    int               id;
    int               obj_type;
    char              name[NAMELEN];

    // position, velocity, and acceleration:
    Camera            cam;
    Point             velocity;
    Point             arcade_velocity;
    Point             accel;
    float             thrust;
    float             trans_x;
    float             trans_y;
    float             trans_z;
    float             drag;

    // attitude and angular velocity:
    float             roll,   pitch,  yaw;
    float             dr,     dp,     dy;
    float             dr_acc, dp_acc, dy_acc;
    float             dr_drg, dp_drg, dy_drg;

    float             flight_path_yaw;
    float             flight_path_pitch;

    // gravitation:
    Point             primary_loc;
    double            primary_mass;

    // aerodynamics:
    float             g_accel;    // acceleration due to gravity (constant)
    float             Do;         // atmospheric density at sea level
    float             CL;         // base coefficient of lift
    float             CD;         // base coefficient of drag
    float             alpha;      // current angle of attack (radians)
    float             stall;      // stall angle of attack (radians)
    bool              lat_thrust; // lateral thrusters enabled in aero mode?
    bool              straight;

    // vibration:
    float             shake;
    Point             vibration;

    // scale factors for ApplyXxx():
    float             roll_rate, pitch_rate, yaw_rate;

    // physical properties:
    double            life;
    float             radius;
    float             mass;
    float             integrity;

    // graphic representation:
    Graphic*          rep;
    Light*            light;

    // AI or human controller:
    Director*         dir;        // null implies an autonomous object

    static double sub_frame;
};

// +--------------------------------------------------------------------+

#endif Physical_h

