/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

(c) 2009, 2010 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

This file is part of the PIXHAWK project

    PIXHAWK is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIXHAWK is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Definition of Unmanned Aerial Vehicle object
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#ifndef _UAS_H_
#define _UAS_H_

#include "UASInterface.h"
#include "MG.h"
#include <mavlink.h>

/**
 * @brief A generic MAVLINK-connected MAV/UAV
 *
 * This class represents one vehicle. It can be used like the real vehicle, e.g. a call to halt()
 * will automatically send the appropriate messages to the vehicle. The vehicle state will also be
 * automatically updated by the comm architecture, so when writing code to e.g. control the vehicle
 * no knowledge of the communication infrastructure is needed.
 */
class UAS : public UASInterface {
    Q_OBJECT
public:
    UAS(int id);
    ~UAS();

    enum BatteryType {
        NICD = 0,
        NIMH = 1,
        LIION = 2,
        LIPOLY = 3,
        LIFE = 4,
        AGZN = 5
           }; ///< The type of battery used

    static const int lipoFull = 4.2f;  ///< 100% charged voltage
    static const int lipoEmpty = 3.5f; ///< Discharged voltage

    /* MANAGEMENT */

    /** @brief The name of the robot */
    QString getUASName(void);
    /** @brief Get the unique system id */
    int getUASID();
    /** @brief The time interval the robot is switched on */
    quint64 getUptime();
    /** @brief Get the status flag for the communication */
    int getCommunicationStatus();
    /** @brief Get low-passed voltage */
    float filterVoltage();
    /** @brief Add one measurement and get low-passed voltage */
    float filterVoltage(float value);
    /** @brief Get the links associated with this robot */
    QList<LinkInterface*>* getLinks();

protected:
    int type;
    quint64 startTime;            ///< The time the UAS was switched on
    CommStatus commStatus;        ///< Communication status
    int uasId;                    ///< Unique system ID
    QString name;                 ///< Human-friendly name of the vehicle, e.g. bravo
    QList<LinkInterface*>* links; ///< List of links this UAS can be reached by
    BatteryType batteryType;      ///< The battery type
    int cells;                    ///< Number of cells

    QList<double> actuatorValues;
    QList<QString> actuatorNames;

    QList<double> motorValues;
    QList<QString> motorNames;
    QList<int> unknownPackets;  ///< Packet IDs which are unknown and have been received

    double thrustSum;           ///< Sum of forward/up thrust of all thrust actuators, in Newtons
    double thrustMax;           ///< Maximum forward/up thrust of this vehicle, in Newtons

    // Battery stats
    double fullVoltage;         ///< Voltage of the fully charged battery (100%)
    double emptyVoltage;        ///< Voltage of the empty battery (0%)
    double startVoltage;        ///< Voltage at system start
    double currentVoltage;      ///< Voltage currently measured
    float lpVoltage;            ///< Low-pass filtered voltage
    int timeRemaining;          ///< Remaining time calculated based on previous and current
    int mode;                   ///< The current mode of the MAV
    int status;                 ///< The current status of the MAV
    quint64 onboardTimeOffset;

    bool controlRollManual;     ///< status flag, true if roll is controlled manually
    bool controlPitchManual;    ///< status flag, true if pitch is controlled manually
    bool controlYawManual;      ///< status flag, true if yaw is controlled manually
    bool controlThrustManual;   ///< status flag, true if thrust is controlled manually

    double manualRollAngle;     ///< Roll angle set by human pilot (radians)
    double manualPitchAngle;    ///< Pitch angle set by human pilot (radians)
    double manualYawAngle;      ///< Yaw angle set by human pilot (radians)
    double manualThrust;        ///< Thrust set by human pilot (radians)
    float receiveDropRate;      ///< Percentage of packets that were dropped on the MAV's receiving link (from GCS and other MAVs)
    float sendDropRate;         ///< Percentage of packets that were not received from the MAV by the GCS

    /** @brief Set the current battery type */
    void setBattery(BatteryType type, int cells);
    /** @brief Estimate how much flight time is remaining */
    int calculateTimeRemaining();
    /** @brief Get the current charge level */
    double getChargeLevel();
    /** @brief Get the human-readable status message for this code */
    void getStatusForCode(int statusCode, QString& uasState, QString& stateDescription);
    /** @brief Check if vehicle is in autonomous mode */
    bool isAuto();

public slots:
    /** @brief Launches the system **/
    void launch();
    void setWaypoint(Waypoint* wp);
    void setWaypointActive(int id);
    /** @brief Order the robot to return home / to land on the runway **/
    void home();
    void halt();
    void go();
    /** @brief Stops the robot system. If it is an MAV, the robot starts the emergency landing procedure **/
    void emergencySTOP();

    /** @brief Kills the robot. All systems are immediately shut down (e.g. the main power line is cut). This might lead to a crash **/
    bool emergencyKILL();

    /** @brief Shut the system cleanly down. Will shut down any onboard computers **/
    void shutdown();

    void requestWaypoints();
    void clearWaypointList();
    /** @brief Enable the motors */
    void enable_motors();
    /** @brief Disable the motors */
    void disable_motors();

    /** @brief Set the values for the manual control of the vehicle */
    void setManualControlCommands(double roll, double pitch, double yaw, double thrust);
    /** @brief Receive a button pressed event from an input device, e.g. joystick */
    void receiveButton(int buttonIndex);

    /** @brief Add a link associated with this robot */
    void addLink(LinkInterface* link);

    /** @brief Receive a message from one of the communication links. */
    void receiveMessage(LinkInterface* link, mavlink_message_t message);

    /** @brief Send a message over this link (to this or to all UAS on this link) */
    void sendMessage(LinkInterface* link, mavlink_message_t message);
    /** @brief Send a message over all links this UAS can be reached with (!= all links) */
    void sendMessage(mavlink_message_t message);

    /** @brief Set this UAS as the system currently in focus, e.g. in the main display widgets */
    void setSelected();

    /** @brief Set current mode of operation, e.g. auto or manual */
    void setMode(int mode);

signals:

    /** @brief The main/battery voltage has changed/was updated */
    void voltageChanged(int uasId, double voltage);
    /** @brief An actuator value has changed */
    void actuatorChanged(UASInterface*, int actId, double value);
    /** @brief An actuator value has changed */
    void actuatorChanged(UASInterface* uas, QString actuatorName, double min, double max, double value);
    void motorChanged(UASInterface* uas, QString motorName, double min, double max, double value);
    /** @brief The system load (MCU/CPU usage) changed */
    void loadChanged(UASInterface* uas, double load);
    /** @brief Propagate a heartbeat received from the system */
    void heartbeat(UASInterface* uas);
};


#endif // _UAS_H_
