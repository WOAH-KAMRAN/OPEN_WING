import React from 'react';

const Aircraft = ({ roll = 0, pitch = 0, heading = 0 }) => {
  const rollDeg = (roll * 180 / Math.PI) || 0;

  return (
    <div
      className="aircraft-svg-wrapper"
      style={{ transform: `rotate(${-rollDeg}deg)` }}
    >
      <svg width="160" height="160" viewBox="-80 -80 160 160">
        <defs>
          <filter id="glow">
            <feGaussianBlur stdDeviation="1.5" result="blur" />
            <feMerge>
              <feMergeNode in="blur" />
              <feMergeNode in="SourceGraphic" />
            </feMerge>
          </filter>
          <linearGradient id="wingGrad" x1="0%" y1="0%" x2="100%" y2="0%">
            <stop offset="0%" stopColor="#0c1830" />
            <stop offset="50%" stopColor="#142240" />
            <stop offset="100%" stopColor="#0c1830" />
          </linearGradient>
        </defs>

        {/* Fuselage body */}
        <ellipse cx="0" cy="0" rx="8" ry="28" fill="none" stroke="rgba(0,212,255,0.4)" strokeWidth="1.5" />

        {/* Fuselage fill */}
        <ellipse cx="0" cy="0" rx="6" ry="26" fill="#0a1420" stroke="rgba(0,212,255,0.15)" strokeWidth="0.5" />

        {/* Delta wing — left */}
        <path
          d="M-2,-15 L-60,25 Q-62,28 -58,28 L-6,10 Z"
          fill="url(#wingGrad)"
          stroke="rgba(0,212,255,0.35)"
          strokeWidth="1"
        />

        {/* Delta wing — right */}
        <path
          d="M2,-15 L60,25 Q62,28 58,28 L6,10 Z"
          fill="url(#wingGrad)"
          stroke="rgba(0,212,255,0.35)"
          strokeWidth="1"
        />

        {/* Wing tip markers */}
        <circle cx="-58" cy="26" r="1.5" fill="rgba(0,212,255,0.6)" />
        <circle cx="58" cy="26" r="1.5" fill="rgba(0,212,255,0.6)" />

        {/* V-tail — left */}
        <path
          d="M-4,22 L-22,46 Q-24,48 -21,48 L-3,26 Z"
          fill="none"
          stroke="rgba(0,212,255,0.3)"
          strokeWidth="1"
        />

        {/* V-tail — right */}
        <path
          d="M4,22 L22,46 Q24,48 21,48 L3,26 Z"
          fill="none"
          stroke="rgba(0,212,255,0.3)"
          strokeWidth="1"
        />

        {/* Nose cone */}
        <path
          d="M-5,-26 Q0,-36 5,-26 Z"
          fill="rgba(0,212,255,0.2)"
          stroke="rgba(0,212,255,0.4)"
          strokeWidth="1"
        />

        {/* Propeller hub (rear) */}
        <circle cx="0" cy="28" r="3" fill="rgba(0,212,255,0.15)" stroke="rgba(0,212,255,0.3)" strokeWidth="0.8" />

        {/* Center line */}
        <line x1="0" y1="-24" x2="0" y2="24" stroke="rgba(0,212,255,0.08)" strokeWidth="0.5" />

        {/* Wing root highlights */}
        <line x1="-2" y1="-10" x2="-35" y2="18" stroke="rgba(0,212,255,0.08)" strokeWidth="0.5" />
        <line x1="2" y1="-10" x2="35" y2="18" stroke="rgba(0,212,255,0.08)" strokeWidth="0.5" />

        {/* Direction indicator (nose forward) */}
        <line x1="0" y1="-32" x2="0" y2="-28" stroke="var(--cyan)" strokeWidth="1.5" opacity="0.6" />
        <line x1="0" y1="-34" x2="0" y2="-32" stroke="var(--cyan)" strokeWidth="0.8" opacity="0.4" />

        {/* Roll indication glow on wingtips */}
        <circle cx="-54" cy="22" r="4" fill="rgba(0,212,255,0.04)" />
        <circle cx="54" cy="22" r="4" fill="rgba(0,212,255,0.04)" />
      </svg>
    </div>
  );
};

export default Aircraft;
