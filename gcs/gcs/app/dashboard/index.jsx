import React from 'react';
import Horizon from '../../widgets/horizon';
import Compass from '../../widgets/compass';
import Aircraft from '../../widgets/aircraft';
import Gauge from '../../widgets/gauge';

const Dashboard = ({ state }) => {
  const { attitude, heading, gps, battery, flightMode, status, rcChannels } = state;

  const rollDeg = (attitude.roll || 0) * 180 / Math.PI;
  const pitchDeg = (attitude.pitch || 0) * 180 / Math.PI;
  const yawDeg = (attitude.yaw || 0) * 180 / Math.PI;
  const headingDeg = heading.heading || 0;

  const modeNames = {
    0: 'MANUAL', 1: 'STABILIZE', 2: 'AUTO', 3: 'RTL'
  };

  const batPct = battery.percentage || 0;
  const batColor = batPct > 50 ? 'green' : batPct > 20 ? 'orange' : 'red';
  const batV = battery.voltage || 0;
  const batA = battery.current || 0;

  const throttle = rcChannels?.channels?.[2] || 0;
  const throttlePct = Math.round(((throttle - 1000) / 1000) * 100);

  return (
    <div className="page-enter">
      <div className="section-title">Flight Dashboard</div>

      <div className="dashboard-layout">
        {/* === LEFT PANEL: Aircraft + Instruments === */}
        <div className="dashboard-left">
          <div className="frame anim-slide-up">
            <div className="frame-title">Aircraft Attitude</div>
            <div className="aircraft-card">
              <div className="ring-outer" />
              <div className="ring-mid" />
              <div className="ring-inner" />
              <Aircraft roll={attitude.roll || 0} pitch={attitude.pitch || 0} heading={headingDeg} />
            </div>
          </div>

          <div className="dashboard-instruments anim-slide-up">
            <div className="frame" style={{ textAlign: 'center', padding: '16px' }}>
              <div className="frame-title">Horizon</div>
              <Horizon roll={attitude.roll || 0} pitch={attitude.pitch || 0} />
            </div>
            <div className="frame" style={{ textAlign: 'center', padding: '16px' }}>
              <div className="frame-title">Compass</div>
              <Compass heading={headingDeg} />
            </div>
          </div>
        </div>

        {/* === RIGHT PANEL: Telemetry Data === */}
        <div className="dashboard-right">
          <div className="grid-2 anim-slide-up">
            {/* Attitude */}
            <div className="frame">
              <div className="frame-title">Attitude</div>
              <div className="data-row">
                <span className="data-label">Roll</span>
                <span className="data-value text-orange">{rollDeg.toFixed(1)}°</span>
              </div>
              <div className="data-row">
                <span className="data-label">Pitch</span>
                <span className="data-value text-cyan">{pitchDeg.toFixed(1)}°</span>
              </div>
              <div className="data-row">
                <span className="data-label">Yaw</span>
                <span className="data-value text-green">{yawDeg.toFixed(1)}°</span>
              </div>
            </div>

            {/* GPS */}
            <div className="frame">
              <div className="frame-title">GPS</div>
              <div className="data-row">
                <span className="data-label">Lat</span>
                <span className="data-value mono" style={{ fontSize: '11px' }}>
                  {(gps.latitude || 0).toFixed(6)}
                </span>
              </div>
              <div className="data-row">
                <span className="data-label">Lon</span>
                <span className="data-value mono" style={{ fontSize: '11px' }}>
                  {(gps.longitude || 0).toFixed(6)}
                </span>
              </div>
              <div className="data-row">
                <span className="data-label">Alt</span>
                <span className="data-value">{(gps.altitude || 0).toFixed(1)}m</span>
              </div>
              <div className="data-row">
                <span className="data-label">Speed</span>
                <span className="data-value">{(gps.speed || 0).toFixed(1)}m/s</span>
              </div>
              <div className="data-row">
                <span className="data-label">Sats</span>
                <span className="data-value">{gps.satellites || 0}</span>
              </div>
            </div>
          </div>

          <div className="grid-2 anim-slide-up">
            {/* Flight Mode */}
            <div className="frame">
              <div className="frame-title">Flight Mode</div>
              <div style={{ textAlign: 'center', padding: '12px 0', display: 'flex', flexDirection: 'column', gap: '10px' }}>
                <span className={`badge ${flightMode.armed ? 'badge-red' : 'badge-green'}`}
                      style={{ fontSize: '14px', padding: '6px 24px', alignSelf: 'center' }}>
                  {modeNames[flightMode.mode] || 'UNKNOWN'}
                </span>
                <span className={`data-value ${flightMode.armed ? 'text-red' : 'text-green'}`}
                      style={{ textAlign: 'center', fontSize: '12px' }}>
                  {flightMode.armed ? 'ARMED' : 'DISARMED'}
                </span>
              </div>
              <div className="data-row">
                <span className="data-label">Throttle</span>
                <span className="data-value">{throttlePct}%</span>
              </div>
            </div>

            {/* Battery */}
            <div className="frame">
              <div className="frame-title">Battery</div>
              <div style={{ display: 'flex', justifyContent: 'center' }}>
                <Gauge value={batV} min={0} max={16.8} label="Voltage" units="V" size={100} />
              </div>
              <div className="data-row">
                <span className="data-label">Current</span>
                <span className="data-value">{batA.toFixed(2)}A</span>
              </div>
              <div className="data-row">
                <span className="data-label">Charge</span>
                <span className="data-value">{batPct}%</span>
              </div>
              <div className="progress-bar">
                <div className={`progress-fill ${batColor}`} style={{ width: batPct + '%' }} />
              </div>
            </div>
          </div>

          <div className="grid-2 anim-slide-up">
            {/* System Status */}
            <div className="frame">
              <div className="frame-title">System</div>
              <div className="data-row">
                <span className="data-label">CPU</span>
                <span className="data-value mono">{(status.cpuLoad || 0)}%</span>
              </div>
              <div className="data-row">
                <span className="data-label">Loop</span>
                <span className="data-value mono">{(status.loopTime || 0)}μs</span>
              </div>
              <div className="data-row">
                <span className="data-label">Errors</span>
                <span className={`data-value mono ${(status.errors || 0) > 0 ? 'text-red' : 'text-green'}`}>
                  {status.errors || 0}
                </span>
              </div>
            </div>

            {/* Speed Gauge */}
            <div className="frame">
              <div className="frame-title">Airspeed</div>
              <div style={{ display: 'flex', justifyContent: 'center' }}>
                <Gauge value={(gps.speed || 0) * 3.6} min={0} max={50} label="Speed" units="km/h" size={100} />
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default Dashboard;
