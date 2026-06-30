import React from 'react';
import Compass from '../../widgets/compass';

const CompassView = ({ state }) => {
  const { heading } = state;

  return (
    <div className="card" style={{ textAlign: 'center', padding: '40px 20px' }}>
      <h2 style={{ marginBottom: '24px' }}>Compass</h2>
      <Compass heading={heading.heading || 0} />
      <div className="value-lg mono text-cyan" style={{ marginTop: '20px' }}>
        {(heading.heading || 0).toFixed(1)}°
      </div>
    </div>
  );
};

export default CompassView;
