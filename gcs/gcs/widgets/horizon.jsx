import React, { useRef, useEffect } from 'react';

const Horizon = ({ roll, pitch }) => {
  const canvasRef = useRef(null);
  const W = 280;
  const H = 280;

  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const cx = W / 2;
    const cy = H / 2;
    const R = Math.min(W, H) / 2 - 10;
    const rollDeg = (roll * 180 / Math.PI) || 0;
    const pitchDeg = (pitch * 180 / Math.PI) || 0;
    const pitchPx = -pitchDeg * 3;

    ctx.clearRect(0, 0, W, H);

    // Outer bezel
    ctx.save();
    ctx.beginPath();
    ctx.arc(cx, cy, R, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(0,212,255,0.15)';
    ctx.lineWidth = 3;
    ctx.stroke();

    // Roll scale arc
    for (let deg = -60; deg <= 60; deg += 10) {
      const angle = (-deg - 90) * Math.PI / 180;
      const inner = deg % 30 === 0 ? R - 16 : R - 12;
      const outer = deg % 30 === 0 ? R - 4 : R - 8;
      ctx.beginPath();
      ctx.moveTo(cx + Math.cos(angle) * inner, cy + Math.sin(angle) * inner);
      ctx.lineTo(cx + Math.cos(angle) * outer, cy + Math.sin(angle) * outer);
      ctx.strokeStyle = deg % 30 === 0 ? 'rgba(139,148,158,0.6)' : 'rgba(72,79,88,0.4)';
      ctx.lineWidth = deg % 30 === 0 ? 2 : 1;
      ctx.stroke();

      if (deg % 30 === 0 && deg !== 0) {
        ctx.fillStyle = 'rgba(139,148,158,0.5)';
        ctx.font = '9px "JetBrains Mono", monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        const lx = cx + Math.cos(angle) * (R - 26);
        const ly = cy + Math.sin(angle) * (R - 26);
        ctx.fillText(deg + '\u00b0', lx, ly);
      }
    }

    // Clip to inner circle
    ctx.beginPath();
    ctx.arc(cx, cy, R - 6, 0, Math.PI * 2);
    ctx.clip();

    // Sky/ground with roll + pitch
    ctx.save();
    ctx.translate(cx, cy);
    ctx.rotate(-roll);
    ctx.translate(0, pitchPx);

    // Sky gradient
    const skyGrad = ctx.createLinearGradient(0, -H, 0, 0);
    skyGrad.addColorStop(0, '#060e1e');
    skyGrad.addColorStop(0.4, '#0a1a30');
    skyGrad.addColorStop(0.7, '#102540');
    skyGrad.addColorStop(1, '#1a3860');
    ctx.fillStyle = skyGrad;
    ctx.fillRect(-W, -H, W * 2, H);

    // Ground gradient
    const gndGrad = ctx.createLinearGradient(0, 0, 0, H);
    gndGrad.addColorStop(0, '#1e1814');
    gndGrad.addColorStop(0.3, '#14100c');
    gndGrad.addColorStop(0.6, '#0a0806');
    gndGrad.addColorStop(1, '#040302');
    ctx.fillStyle = gndGrad;
    ctx.fillRect(-W, 0, W * 2, H);

    // Horizon divider glow
    const hGrad = ctx.createLinearGradient(0, -3, 0, 3);
    hGrad.addColorStop(0, 'rgba(0,212,255,0)');
    hGrad.addColorStop(0.3, 'rgba(0,212,255,0.08)');
    hGrad.addColorStop(0.5, 'rgba(0,212,255,0.12)');
    hGrad.addColorStop(0.7, 'rgba(0,212,255,0.08)');
    hGrad.addColorStop(1, 'rgba(0,212,255,0)');
    ctx.fillStyle = hGrad;
    ctx.fillRect(-W, -3, W * 2, 6);

    // Pitch ladder
    for (let deg = -90; deg <= 90; deg += 10) {
      if (deg === 0) continue;
      const y = -deg * 3;
      const isMajor = deg % 30 === 0;
      const halfW = isMajor ? 55 : 28;

      ctx.strokeStyle = deg > 0 ? 'rgba(255,255,255,0.2)' : 'rgba(255,255,255,0.35)';
      ctx.lineWidth = isMajor ? 1.5 : 0.8;
      ctx.beginPath();
      ctx.moveTo(-halfW, y);
      ctx.lineTo(halfW, y);
      ctx.stroke();

      if (isMajor) {
        ctx.fillStyle = 'rgba(255,255,255,0.35)';
        ctx.font = '9px "JetBrains Mono", monospace';
        ctx.textAlign = 'right';
        ctx.textBaseline = 'middle';
        ctx.fillText(Math.abs(deg) + '\u00b0', -halfW - 7, y);
        ctx.textAlign = 'left';
        ctx.fillText(Math.abs(deg) + '\u00b0', halfW + 7, y);
      }
    }

    // Horizon line — glowing cyan
    ctx.strokeStyle = '#00d4ff';
    ctx.lineWidth = 2;
    ctx.shadowColor = '#00d4ff';
    ctx.shadowBlur = 12;
    ctx.beginPath();
    ctx.moveTo(-90, 0);
    ctx.lineTo(90, 0);
    ctx.stroke();
    ctx.shadowBlur = 0;

    ctx.restore(); // roll/pitch transform

    // Aircraft reference (fixed center)
    ctx.save();
    ctx.translate(cx, cy);

    // Center dot
    ctx.fillStyle = '#00d4ff';
    ctx.shadowColor = '#00d4ff';
    ctx.shadowBlur = 6;
    ctx.beginPath();
    ctx.arc(0, 0, 2, 0, Math.PI * 2);
    ctx.fill();
    ctx.shadowBlur = 0;

    // Wing lines
    ctx.strokeStyle = '#00d4ff';
    ctx.lineWidth = 1.8;
    ctx.shadowColor = '#00d4ff';
    ctx.shadowBlur = 6;
    ctx.beginPath();
    ctx.moveTo(-40, 0);
    ctx.lineTo(-14, 0);
    ctx.moveTo(40, 0);
    ctx.lineTo(14, 0);
    ctx.stroke();
    ctx.shadowBlur = 0;

    // Chevron
    ctx.strokeStyle = '#00d4ff';
    ctx.lineWidth = 1.8;
    ctx.shadowColor = '#00d4ff';
    ctx.shadowBlur = 4;
    ctx.beginPath();
    ctx.moveTo(0, -34);
    ctx.lineTo(-11, -16);
    ctx.lineTo(11, -16);
    ctx.closePath();
    ctx.stroke();
    ctx.shadowBlur = 0;

    ctx.restore();

    // Roll indicator triangle (top)
    ctx.save();
    const rollAngleRad = (-roll - Math.PI / 2);
    const tipX = cx + Math.cos(rollAngleRad) * (R - 6);
    const tipY = cy + Math.sin(rollAngleRad) * (R - 6);
    ctx.fillStyle = '#ffffff';
    ctx.shadowColor = 'rgba(255,255,255,0.3)';
    ctx.shadowBlur = 4;
    ctx.beginPath();
    ctx.moveTo(tipX, tipY);
    ctx.lineTo(tipX - 5, tipY + 10);
    ctx.lineTo(tipX + 5, tipY + 10);
    ctx.closePath();
    ctx.fill();
    ctx.shadowBlur = 0;
    ctx.restore();

    // Numeric readouts
    ctx.fillStyle = 'rgba(139,148,158,0.6)';
    ctx.font = '9px "JetBrains Mono", monospace';
    ctx.textAlign = 'left';
    ctx.textBaseline = 'bottom';
    ctx.fillText('ROL', 10, H - 16);
    ctx.fillStyle = '#e6edf3';
    ctx.fillText(rollDeg.toFixed(1) + '\u00b0', 44, H - 16);

    ctx.fillStyle = 'rgba(139,148,158,0.6)';
    ctx.textAlign = 'right';
    ctx.fillText('PIT', W - 10, H - 16);
    ctx.fillStyle = '#e6edf3';
    ctx.fillText(pitchDeg.toFixed(1) + '\u00b0', W - 44, H - 16);

  }, [roll, pitch]);

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

export default Horizon;
