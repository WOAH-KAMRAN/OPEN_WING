import React, { useRef, useEffect } from 'react';

const Compass = ({ heading = 0 }) => {
  const canvasRef = useRef(null);
  const W = 200;
  const H = 200;

  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const cx = W / 2;
    const cy = H / 2;
    const R = Math.min(W, H) / 2 - 12;

    ctx.clearRect(0, 0, W, H);

    // Dark filled circle
    ctx.beginPath();
    ctx.arc(cx, cy, R + 3, 0, Math.PI * 2);
    ctx.fillStyle = '#0a0e16';
    ctx.fill();

    // Outer ring glow
    ctx.beginPath();
    ctx.arc(cx, cy, R + 2, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(0,212,255,0.1)';
    ctx.lineWidth = 4;
    ctx.stroke();

    ctx.beginPath();
    ctx.arc(cx, cy, R + 1, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(0,212,255,0.05)';
    ctx.lineWidth = 1;
    ctx.stroke();

    // Rotate by heading
    ctx.save();
    ctx.translate(cx, cy);
    ctx.rotate((heading * Math.PI) / 180);

    // Tick marks
    for (let deg = 0; deg < 360; deg += 5) {
      const angle = (deg - 90) * Math.PI / 180;
      const isMajor = deg % 30 === 0;
      const isMid = deg % 10 === 0;
      let tickLen;
      if (isMajor) tickLen = 16;
      else if (isMid) tickLen = 11;
      else tickLen = 6;

      const x1 = Math.cos(angle) * R;
      const y1 = Math.sin(angle) * R;
      const x2 = Math.cos(angle) * (R - tickLen);
      const y2 = Math.sin(angle) * (R - tickLen);

      ctx.strokeStyle = isMajor ? 'rgba(139,148,158,0.7)' : isMid ? 'rgba(72,79,88,0.5)' : 'rgba(48,54,61,0.4)';
      ctx.lineWidth = isMajor ? 2 : 1;
      ctx.beginPath();
      ctx.moveTo(x1, y1);
      ctx.lineTo(x2, y2);
      ctx.stroke();

      if (isMajor) {
        const lx = Math.cos(angle) * (R - 26);
        const ly = Math.sin(angle) * (R - 26);
        const isCardinal = deg % 90 === 0;
        ctx.fillStyle = isCardinal ? '#00d4ff' : 'rgba(139,148,158,0.6)';
        ctx.font = isCardinal ? 'bold 12px "JetBrains Mono", monospace' : '10px "JetBrains Mono", monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';

        let label;
        if (deg === 0) label = 'N';
        else if (deg === 90) label = 'E';
        else if (deg === 180) label = 'S';
        else if (deg === 270) label = 'W';
        else label = (deg / 10).toString();

        ctx.fillText(label, lx, ly);
      }
    }

    ctx.restore();

    // Pointer triangle (top)
    ctx.save();
    ctx.translate(cx, cy);

    ctx.fillStyle = '#f85149';
    ctx.shadowColor = '#f85149';
    ctx.shadowBlur = 10;
    ctx.beginPath();
    ctx.moveTo(0, -R + 8);
    ctx.lineTo(-8, -R + 22);
    ctx.lineTo(8, -R + 22);
    ctx.closePath();
    ctx.fill();
    ctx.shadowBlur = 0;

    // Inner white tip
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
    ctx.font = 'bold 20px "JetBrains Mono", monospace';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'bottom';
    ctx.shadowColor = 'rgba(0,0,0,0.8)';
    ctx.shadowBlur = 6;
    ctx.fillText(heading.toFixed(0) + '\u00b0', cx, H - 8);
    ctx.shadowBlur = 0;

  }, [heading]);

  return (
    <canvas
      ref={canvasRef}
      width={W}
      height={H}
      style={{
        borderRadius: '50%',
        display: 'block',
        background: '#0d1117',
        boxShadow: '0 0 20px rgba(0,212,255,0.06)'
      }}
    />
  );
};

export default Compass;
