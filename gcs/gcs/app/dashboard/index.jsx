import React from 'react';
import Horizon from '../../widgets/horizon';
import Compass from '../../widgets/compass';

const Dashboard = ({ state }) => {
  const { attitude, heading, gps, battery, flightMode, status } = state;

  const rollDeg = (attitude.roll || 0) * 180 / Math.PI;
  const pitchDeg = (attitude.pitch || 0) * 180 / Math.PI;
  const yawDeg = (attitude.yaw || 0) * 180 / Math.PI;
  const headingDeg = heading.heading || 0;

  const modeNames = {
    0: 'MANUAL', 1: 'STABILIZE', 2: 'AUTO', 3: 'RTL'
  };

  const batPct = battery.percentage || 0;
  const batColor = batPct > 50 ? 'green' : batPct > 20 ? 'orange' : 'red';

  return (
    <div>
      <h2 style={{
        fontSize: '13px', fontWeight: 600, color: '#6e7681',
        textTransform: 'uppercase', letterSpacing: '1px', marginBottom: '12px'
      }}>
        Flight Dashboard
      </h2>

      <div className="grid-2" style={{ marginBottom: '12px' }}>
        <div className="card" style={{ textAlign: 'center', padding: '20px' }}>
          <div className="label" style={{ marginBottom: '8px' }}>Artificial Horizon</div>
          <Horizon roll={attitude.roll || 0} pitch={attitude.pitch || 0} />
        </div>
        <div className="card" style={{ textAlign: 'center', padding: '20px' }}>
          <div className="label" style={{ marginBottom: '8px' }}>Compass</div>
          <Compass heading={headingDeg} />
        </div>
      </div>

      <div className="grid-auto">
        <div className="card">
          <h2>Attitude</h2>
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

        <div className="card">
          <h2>GPS</h2>
          <div className="data-row">
            <span className="data-label">Lat</span>
            <span className="data-value mono" style={{ fontSize: '12px' }}>{(gps.latitude || 0).toFixed(6)}</span>
          </div>
          <div className="data-row">
            <span className="data-label">Lon</span>
            <span className="data-value mono" style={{ fontSize: '12px' }}>{(gps.longitude || 0).toFixed(6)}</span>
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

        <div className="card">
          <h2>Flight Mode</h2>
          <div style={{ textAlign: 'center', padding: '8px 0' }}>
            <span className={`badge ${flightMode.armed ? 'badge-red' : 'badge-green'}`}
                  style={{ fontSize: '16px', padding: '6px 20px' }}>
              {modeNames[flightMode.mode] || 'UNKNOWN'}
            </span>
          </div>
          <div className="data-row">
            <span className="data-label">Armed</span>
            <span className={`data-value ${flightMode.armed ? 'text-red' : 'text-green'}`}>
              {flightMode.armed ? 'YES' : 'NO'}
            </span>
          </div>
        </div>

        <div className="card">
          <h2>Battery</h2>
          <div style={{ textAlign: 'center', padding: '4px 0' }}>
            <span className="mono value-lg text-cyan">{battery.voltage.toFixed(2)}V</span>
          </div>
          <div className="data-row">
            <span className="data-label">Current</span>
            <span className="data-value">{battery.current.toFixed(2)}A</span>
          </div>
          <div className="data-row">
            <span className="data-label">Charge</span>
            <span className="data-value">{batPct}%</span>
          </div>
          <div className="progress-bar">
            <div className={`progress-fill ${batColor}`} style={{ width: batPct + '%' }} />
          </div>
        </div>

        <div className="card">
          <h2>Status</h2>
          <div className="data-row">
            <span className="data-label">CPU Load</span>
            <span className="data-value mono">{(status.cpuLoad || 0)}%</span>
          </div>
          <div className="data-row">
            <span className="data-label">Loop Time</span>
            <span className="data-value mono">{(status.loopTime || 0)}μs</span>
          </div>
          <div className="data-row">
            <span className="data-label">Errors</span>
            <span className={`data-value mono ${status.errors > 0 ? 'text-red' : 'text-green'}`}>
              {status.errors || 0}
            </span>
          </div>
        </div>
      </div>
    </div>
  );
};

export default Dashboard;
