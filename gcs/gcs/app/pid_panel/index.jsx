import React, { useState } from 'react';

const PIDPanel = ({ state, onSetPid, onGetPid }) => {
  const { pidValues } = state;
  const [selectedAxis, setSelectedAxis] = useState('roll');
  const [localPid, setLocalPid] = useState({ p: 0, i: 0, d: 0 });

  const axisNames = { roll: 'Roll', pitch: 'Pitch', yaw: 'Yaw' };
  const axisNumbers = { roll: 0, pitch: 1, yaw: 2 };

  const handleAxisChange = (axis) => {
    setSelectedAxis(axis);
    setLocalPid(pidValues[axis]);
  };

  const handlePidChange = (field, value) => {
    setLocalPid({ ...localPid, [field]: parseFloat(value) || 0 });
  };

  return (
    <div>
      <h2 style={{
        fontSize: '13px', fontWeight: 600, color: '#6e7681',
        textTransform: 'uppercase', letterSpacing: '1px', marginBottom: '12px'
      }}>
        PID Tuning
      </h2>

      <div className="grid-2">
        <div className="card card-highlight">
          <h2>{axisNames[selectedAxis]} PID</h2>

          <div style={{ marginBottom: '8px' }}>
            <label className="label" style={{ display: 'block', marginBottom: '4px' }}>Axis</label>
            <select
              value={selectedAxis}
              onChange={(e) => handleAxisChange(e.target.value)}
              className="select"
              style={{ width: '100%' }}
            >
              <option value="roll">Roll</option>
              <option value="pitch">Pitch</option>
              <option value="yaw">Yaw</option>
            </select>
          </div>

          {['p', 'i', 'd'].map(field => (
            <div key={field} style={{ marginBottom: '10px' }}>
              <label className="label" style={{ display: 'block', marginBottom: '4px' }}>
                {field.toUpperCase()} ({field === 'p' ? 'Proportional' : field === 'i' ? 'Integral' : 'Derivative'})
              </label>
              <input
                type="number"
                step="0.01"
                value={localPid[field]}
                onChange={(e) => handlePidChange(field, e.target.value)}
                className="inp"
              />
            </div>
          ))}

          <div style={{ display: 'flex', gap: '8px', marginTop: '4px' }}>
            <button onClick={() => onGetPid?.(axisNumbers[selectedAxis])} className="btn btn-primary" style={{ flex: 1 }}>
              Read
            </button>
            <button onClick={() => onSetPid?.(axisNumbers[selectedAxis], localPid.p, localPid.i, localPid.d)} className="btn btn-success" style={{ flex: 1 }}>
              Write
            </button>
          </div>
        </div>

        <div className="card">
          <h2>All Values</h2>
          {Object.entries(pidValues).map(([axis, pid]) => (
            <div key={axis} style={{
              background: '#0d1117', border: '1px solid #1c2535',
              borderRadius: '6px', padding: '10px 12px', marginBottom: '8px'
            }}>
              <div className="label" style={{ marginBottom: '6px' }}>{axisNames[axis]}</div>
              <div className="mono" style={{ fontSize: '12px', lineHeight: '1.8' }}>
                <span className="text-cyan">P</span> {pid.p.toFixed(4)}
                {'  '}<span className="text-green">I</span> {pid.i.toFixed(4)}
                {'  '}<span className="text-orange">D</span> {pid.d.toFixed(4)}
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};

export default PIDPanel;
