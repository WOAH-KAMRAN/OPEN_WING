import React, { useRef, useEffect } from 'react';

const Compass = ({ heading }) => {
  const canvasRef = useRef(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const W = canvas.width;
    const H = canvas.height;
    const cx = W / 2;
    const cy = H / 2;
    const R = Math.min(W, H) / 2 - 10;

    ctx.clearRect(0, 0, W, H);

    // Dark filled circle
    ctx.beginPath();
    ctx.arc(cx, cy, R + 4, 0, Math.PI * 2);
    ctx.fillStyle = '#0d1117';
    ctx.fill();

    // Outer ring with glow
    ctx.beginPath();
    ctx.arc(cx, cy, R + 2, 0, Math.PI * 2);
    ctx.strokeStyle = '#1c2535';
    ctx.lineWidth = 4;
    ctx.stroke();

    ctx.beginPath();
    ctx.arc(cx, cy, R + 1, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(0, 212, 255, 0.15)';
    ctx.lineWidth = 1;
    ctx.stroke();

    // Rotate by heading
    ctx.save();
    ctx.translate(cx, cy);
    ctx.rotate((heading * Math.PI) / 180);

    // Tick marks — every 5 degrees
    for (let deg = 0; deg < 360; deg += 5) {
      const angle = (deg - 90) * Math.PI / 180;
      const isMajor = deg % 30 === 0;
      const isMid = deg % 10 === 0;
      let tickLen;
      if (isMajor) tickLen = 14;
      else if (isMid) tickLen = 10;
      else tickLen = 5;

      const x1 = Math.cos(angle) * R;
      const y1 = Math.sin(angle) * R;
      const x2 = Math.cos(angle) * (R - tickLen);
      const y2 = Math.sin(angle) * (R - tickLen);

      ctx.strokeStyle = isMajor ? '#8b949e' : isMid ? '#484f58' : '#30363d';
      ctx.lineWidth = isMajor ? 2 : 1;
      ctx.beginPath();
      ctx.moveTo(x1, y1);
      ctx.lineTo(x2, y2);
      ctx.stroke();

      // Degree labels on major ticks
      if (isMajor) {
        const lx = Math.cos(angle) * (R - 22);
        const ly = Math.sin(angle) * (R - 22);
        ctx.fillStyle = deg === 0 ? '#00d4ff' : '#8b949e';
        ctx.font = deg === 0 ? 'bold 11px monospace' : '10px monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        const label = deg === 0 ? 'N' : deg === 90 ? 'E' : deg === 180 ? 'S' : deg === 270 ? 'W' : (deg / 10).toString();
        ctx.fillText(label, lx, ly);
      }
    }

    ctx.restore();

    // Pointer triangle (red/white)
    ctx.save();
    ctx.translate(cx, cy);

    ctx.fillStyle = '#f85149';
    ctx.shadowColor = '#f85149';
    ctx.shadowBlur = 8;
    ctx.beginPath();
    ctx.moveTo(0, -R + 8);
    ctx.lineTo(-7, -R + 20);
    ctx.lineTo(7, -R + 20);
    ctx.closePath();
    ctx.fill();
    ctx.shadowBlur = 0;

    ctx.fillStyle = '#ffffff';
    ctx.beginPath();
    ctx.moveTo(0, -R + 12);
    ctx.lineTo(-3, -R + 18);
    ctx.lineTo(3, -R + 18);
    ctx.closePath();
    ctx.fill();

    ctx.restore();

    // Heading readout at bottom
    ctx.fillStyle = '#e6edf3';
    ctx.font = 'bold 18px monospace';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'bottom';
    ctx.shadowColor = 'rgba(0,0,0,0.8)';
    ctx.shadowBlur = 4;
    ctx.fillText(heading.toFixed(0) + '\u00b0', cx, H - 6);
    ctx.shadowBlur = 0;
  }, [heading]);

  return (
    <canvas
      ref={canvasRef}
      width={150}
      height={150}
      className="card"
      style={{ borderRadius: '50%', display: 'block' }}
    />
  );
};

export default Compass;
