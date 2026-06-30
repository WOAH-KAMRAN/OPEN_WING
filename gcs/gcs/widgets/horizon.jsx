import React, { useRef, useEffect } from 'react';

const Horizon = ({ roll, pitch }) => {
  const canvasRef = useRef(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const W = canvas.width;
    const H = canvas.height;
    const cx = W / 2;
    const cy = H / 2;
    const R = Math.min(W, H) / 2 - 6;

    ctx.clearRect(0, 0, W, H);

    // Outer bezel ring
    ctx.save();
    ctx.beginPath();
    ctx.arc(cx, cy, R, 0, Math.PI * 2);
    ctx.strokeStyle = '#1c2535';
    ctx.lineWidth = 4;
    ctx.stroke();

    // Roll scale arc (top half)
    for (let deg = -60; deg <= 60; deg += 10) {
      const angle = (-deg - 90) * Math.PI / 180;
      const inner = deg % 30 === 0 ? R - 16 : R - 12;
      const outer = deg % 30 === 0 ? R - 4 : R - 8;
      ctx.beginPath();
      ctx.moveTo(cx + Math.cos(angle) * inner, cy + Math.sin(angle) * inner);
      ctx.lineTo(cx + Math.cos(angle) * outer, cy + Math.sin(angle) * outer);
      ctx.strokeStyle = deg % 30 === 0 ? '#8b949e' : '#484f58';
      ctx.lineWidth = deg % 30 === 0 ? 2 : 1;
      ctx.stroke();

      if (deg % 30 === 0 && deg !== 0) {
        ctx.fillStyle = '#8b949e';
        ctx.font = '10px monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        const lx = cx + Math.cos(angle) * (R - 24);
        const ly = cy + Math.sin(angle) * (R - 24);
        ctx.fillText(deg + '\u00b0', lx, ly);
      }
    }

    // Clip to inner circle
    ctx.beginPath();
    ctx.arc(cx, cy, R - 5, 0, Math.PI * 2);
    ctx.clip();

    // Sky/ground with roll rotation and pitch translation
    ctx.save();
    ctx.translate(cx, cy);
    ctx.rotate(-roll);
    const pitchPx = pitch * 80;
    ctx.translate(0, pitchPx);

    // Sky gradient
    const skyGrad = ctx.createLinearGradient(0, -H, 0, 0);
    skyGrad.addColorStop(0, '#0a1628');
    skyGrad.addColorStop(0.5, '#0d2137');
    skyGrad.addColorStop(1, '#1a3050');
    ctx.fillStyle = skyGrad;
    ctx.fillRect(-W, -H, W * 2, H);

    // Ground gradient
    const gndGrad = ctx.createLinearGradient(0, 0, 0, H);
    gndGrad.addColorStop(0, '#1a1410');
    gndGrad.addColorStop(0.5, '#0d0b08');
    gndGrad.addColorStop(1, '#050403');
    ctx.fillStyle = gndGrad;
    ctx.fillRect(-W, 0, W * 2, H);

    // Pitch ladder
    for (let deg = -90; deg <= 90; deg += 10) {
      if (deg === 0) continue;
      const y = -deg * 80 / 90 * 90;
      const isMajor = deg % 30 === 0;
      const halfW = isMajor ? 50 : 25;

      ctx.strokeStyle = deg > 0 ? 'rgba(255,255,255,0.3)' : 'rgba(255,255,255,0.5)';
      ctx.lineWidth = isMajor ? 1.5 : 1;
      ctx.beginPath();
      ctx.moveTo(-halfW, y);
      ctx.lineTo(halfW, y);
      ctx.stroke();

      if (isMajor) {
        ctx.fillStyle = 'rgba(255,255,255,0.5)';
        ctx.font = '10px monospace';
        ctx.textAlign = 'right';
        ctx.textBaseline = 'middle';
        ctx.fillText(Math.abs(deg) + '\u00b0', -halfW - 6, y);
        ctx.textAlign = 'left';
        ctx.fillText(Math.abs(deg) + '\u00b0', halfW + 6, y);
      }
    }

    // Center horizon line (glowing cyan)
    ctx.strokeStyle = '#00d4ff';
    ctx.lineWidth = 2.5;
    ctx.shadowColor = '#00d4ff';
    ctx.shadowBlur = 10;
    ctx.beginPath();
    ctx.moveTo(-80, 0);
    ctx.lineTo(80, 0);
    ctx.stroke();
    ctx.shadowBlur = 0;

    ctx.restore(); // roll/pitch transform

    // Aircraft reference (fixed center — chevron)
    ctx.save();
    ctx.translate(cx, cy);

    // Center dot
    ctx.fillStyle = '#00d4ff';
    ctx.shadowColor = '#00d4ff';
    ctx.shadowBlur = 6;
    ctx.beginPath();
    ctx.arc(0, 0, 2.5, 0, Math.PI * 2);
    ctx.fill();
    ctx.shadowBlur = 0;

    // Wing lines
    ctx.strokeStyle = '#00d4ff';
    ctx.lineWidth = 2;
    ctx.shadowColor = '#00d4ff';
    ctx.shadowBlur = 6;
    ctx.beginPath();
    ctx.moveTo(-35, 0);
    ctx.lineTo(-12, 0);
    ctx.moveTo(35, 0);
    ctx.lineTo(12, 0);
    ctx.stroke();
    ctx.shadowBlur = 0;

    // Chevron top
    ctx.strokeStyle = '#00d4ff';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(0, -30);
    ctx.lineTo(-10, -14);
    ctx.lineTo(10, -14);
    ctx.closePath();
    ctx.stroke();

    ctx.restore();

    // Roll indicator triangle (top of bezel)
    ctx.save();
    const rollAngle = (-roll * Math.PI / 180) - Math.PI / 2;
    const tipX = cx + Math.cos(rollAngle) * (R - 5);
    const tipY = cy + Math.sin(rollAngle) * (R - 5);
    ctx.fillStyle = '#ffffff';
    ctx.beginPath();
    ctx.moveTo(tipX, tipY);
    ctx.lineTo(tipX - 5, tipY + 10);
    ctx.lineTo(tipX + 5, tipY + 10);
    ctx.closePath();
    ctx.fill();
    ctx.restore();

    // Numeric readouts at bottom
    ctx.fillStyle = '#8b949e';
    ctx.font = '10px monospace';
    ctx.textAlign = 'left';
    ctx.textBaseline = 'bottom';
    ctx.fillText('ROL', 8, H - 14);
    ctx.fillStyle = '#e6edf3';
    ctx.fillText((roll * 180 / Math.PI).toFixed(1) + '\u00b0', 38, H - 14);

    ctx.fillStyle = '#8b949e';
    ctx.textAlign = 'right';
    ctx.fillText('PIT', W - 8, H - 14);
    ctx.fillStyle = '#e6edf3';
    ctx.fillText((pitch * 180 / Math.PI).toFixed(1) + '\u00b0', W - 38, H - 14);
  }, [roll, pitch]);

  return (
    <canvas
      ref={canvasRef}
      width={200}
      height={200}
      className="card"
      style={{ borderRadius: '50%', display: 'block' }}
    />
  );
};

export default Horizon;
