import React, { useEffect, useRef } from 'react';
import L from 'leaflet';

const MapView = ({ state }) => {
  const { gps } = state;
  const mapRef = useRef(null);
  const mapInstanceRef = useRef(null);
  const markerRef = useRef(null);
  const pathRef = useRef(null);
  const pathPointsRef = useRef([]);

  useEffect(() => {
    if (!mapRef.current || mapInstanceRef.current) return;

    mapInstanceRef.current = L.map(mapRef.current, {
      zoomControl: false,
      attributionControl: false
    }).setView([0, 0], 2);

    L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
      attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OSM</a> &copy; <a href="https://carto.com/">CARTO</a>'
    }).addTo(mapInstanceRef.current);

    L.control.zoom({ position: 'bottomright' }).addTo(mapInstanceRef.current);

    const icon = L.divIcon({
      html: '<div style="background:#00d4ff;width:12px;height:12px;border-radius:50%;border:2px solid #fff;box-shadow:0 0 10px rgba(0,212,255,0.5)"></div>',
      iconSize: [12, 12],
      iconAnchor: [6, 6],
      className: ''
    });
    markerRef.current = L.marker([0, 0], { icon }).addTo(mapInstanceRef.current);

    pathRef.current = L.polyline([], {
      color: '#00d4ff',
      weight: 2,
      opacity: 0.7
    }).addTo(mapInstanceRef.current);

    return () => {
      if (mapInstanceRef.current) {
        mapInstanceRef.current.remove();
        mapInstanceRef.current = null;
      }
    };
  }, []);

  useEffect(() => {
    if (!mapInstanceRef.current || !gps || gps.fixType === 0) return;

    const lat = gps.latitude;
    const lon = gps.longitude;

    if (markerRef.current) {
      markerRef.current.setLatLng([lat, lon]);
    }

    pathPointsRef.current.push([lat, lon]);
    if (pathPointsRef.current.length > 1000) {
      pathPointsRef.current.shift();
    }

    if (pathRef.current) {
      pathRef.current.setLatLngs(pathPointsRef.current);
    }

    mapInstanceRef.current.setView([lat, lon], 16);
  }, [gps]);

  return (
    <div>
      <h2 style={{
        fontSize: '13px', fontWeight: 600, color: '#6e7681',
        textTransform: 'uppercase', letterSpacing: '1px', marginBottom: '12px'
      }}>
        Map View
      </h2>
      <div className="card" style={{ padding: 0, overflow: 'hidden', position: 'relative' }}>
        <div
          ref={mapRef}
          style={{ height: '500px', width: '100%' }}
        />
        {gps && gps.fixType > 0 && (
          <div className="overlay">
            <div className="data-row">
              <span className="data-label">LAT</span>
              <span className="data-value mono">{gps.latitude.toFixed(6)}</span>
            </div>
            <div className="data-row">
              <span className="data-label">LON</span>
              <span className="data-value mono">{gps.longitude.toFixed(6)}</span>
            </div>
            <div className="data-row">
              <span className="data-label">ALT</span>
              <span className="data-value">{gps.altitude.toFixed(1)}m</span>
            </div>
            <div className="data-row">
              <span className="data-label">SPD</span>
              <span className="data-value">{(gps.speed || 0).toFixed(1)}m/s</span>
            </div>
            <div className="data-row">
              <span className="data-label">SAT</span>
              <span className="data-value">{gps.satellites || 0}</span>
            </div>
            <div className="data-row">
              <span className="data-label">FIX</span>
              <span className="data-value">
                {gps.fixType === 3 ? '3D' : gps.fixType === 2 ? '2D' : gps.fixType === 1 ? '2D' : 'NONE'}
              </span>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default MapView;
