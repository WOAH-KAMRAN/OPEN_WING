import React, { useRef, useEffect } from 'react';

const Gauge = ({ value = 0, min = 0, max = 100, label = '', units = '', size = 120 }) => {
  const canvasRef = useRef(null);
  const valRef = useRef(value);

  useEffect(() => {
    valRef.current = value;
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const W = canvas.width;
    const H = canvas.height;
    const cx = W / 2;
    const cy = H / 2 + 4;
    const R = Math.min(W, H) / 2 - 14;
    const arcStart = Math.PI * 0.75;
    const arcEnd = Math.PI * 2.25;
    const arcRange = arcEnd - arcStart;
    const pct = Math.max(0, Math.min(1, (value - min) / (max - min)));
    const angle = arcStart + pct * arcRange;

    ctx.clearRect(0, 0, W, H);

    // Background arc
    ctx.beginPath();
    ctx.arc(cx, cy, R, arcStart, arcEnd);
    ctx.strokeStyle = 'rgba(255,255,255,0.06)';
    ctx.lineWidth = 6;
    ctx.lineCap = 'round';
    ctx.stroke();

    // Gradient fill arc
    const grad = ctx.createLinearGradient(0, 0, W, 0);
    const p = pct;
    if (p < 0.5) {
      grad.addColorStop(0, 'rgba(0,212,255,0.6)');
      grad.addColorStop(1, 'rgba(63,185,80,0.6)');
    } else if (p < 0.75) {
      grad.addColorStop(0, 'rgba(63,185,80,0.6)');
      grad.addColorStop(1, 'rgba(240,136,62,0.8)');
    } else {
      grad.addColorStop(0, 'rgba(240,136,62,0.8)');
      grad.addColorStop(1, 'rgba(248,81,73,0.9)');
    }

    ctx.beginPath();
    ctx.arc(cx, cy, R, arcStart, angle);
    ctx.strokeStyle = grad;
    ctx.lineWidth = 6;
    ctx.lineCap = 'round';
    ctx.shadowColor = p > 0.75 ? 'rgba(248,81,73,0.4)' : 'rgba(0,212,255,0.3)';
    ctx.shadowBlur = 8;
    ctx.stroke();
    ctx.shadowBlur = 0;

    // Needle
    ctx.save();
    ctx.translate(cx, cy);
    ctx.rotate(angle);
    ctx.beginPath();
    ctx.moveTo(0, -R + 10);
    ctx.lineTo(-3, -4);
    ctx.lineTo(3, -4);
    ctx.closePath();
    ctx.fillStyle = p > 0.75 ? 'var(--red)' : p > 0.5 ? 'var(--orange)' : 'var(--cyan)';
    ctx.shadowColor = p > 0.75 ? 'rgba(248,81,73,0.5)' : 'rgba(0,212,255,0.4)';
    ctx.shadowBlur = 6;
    ctx.fill();
    ctx.shadowBlur = 0;

    // Center dot
    ctx.beginPath();
    ctx.arc(0, 0, 3, 0, Math.PI * 2);
    ctx.fillStyle = 'rgba(255,255,255,0.3)';
    ctx.fill();

    ctx.restore();

    // Value text
    const display = Number.isInteger(value) ? value.toString() : value.toFixed(1);
    ctx.fillStyle = '#e6edf3';
    ctx.font = 'bold 16px "JetBrains Mono", monospace';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(display, cx, cy - 6);

    if (units) {
      ctx.fillStyle = '#6e7681';
      ctx.font = '9px Inter, sans-serif';
      ctx.fillText(units, cx, cy + 12);
    }

    // Tick marks at major intervals
    for (let i = 0; i <= 4; i++) {
      const t = i / 4;
      const a = arcStart + t * arcRange;
      const inner = R - 10;
      const outer = R - 4;
      ctx.beginPath();
      ctx.moveTo(cx + Math.cos(a) * inner, cy + Math.sin(a) * inner);
      ctx.lineTo(cx + Math.cos(a) * outer, cy + Math.sin(a) * outer);
      ctx.strokeStyle = 'rgba(255,255,255,0.1)';
      ctx.lineWidth = 1;
      ctx.stroke();
    }
  }, [value, min, max]);

  return (
    <canvas
      ref={canvasRef}
      width={size}
      height={size}
    />
  );
};

export default Gauge;
