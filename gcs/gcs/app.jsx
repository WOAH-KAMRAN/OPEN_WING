import React, { useState, useEffect } from 'react';
import { createRoot } from 'react-dom/client';
import SerialConnection from './communication/serial';
import WebSocketConnection from './communication/websocket';
import ProtocolParser from './telems/protocol_parser';
import StateManager from './telems/state_manager';
import Dashboard from './app/dashboard/index';
import PIDPanel from './app/pid_panel/index';
import CompassView from './app/compass/index';
import MapView from './app/map/index';
import './styles.css';

const App = () => {
  const [activeTab, setActiveTab] = useState('dashboard');
  const [connectionType, setConnectionType] = useState('serial');
  const [connected, setConnected] = useState(false);
  const [wifiUrl, setWifiUrl] = useState('ws://192.168.4.1:8080');

  const stateManagerRef = React.useRef(new StateManager());
  const parserRef = React.useRef(new ProtocolParser('firmware'));
  const serialRef = React.useRef(null);
  const wsRef = React.useRef(null);

  const state = stateManagerRef.current.getState();

  useEffect(() => {
    serialRef.current = new SerialConnection(handleData, handleError);
    wsRef.current = new WebSocketConnection(handleData, handleError);

    const unsubscribe = stateManagerRef.current.subscribe((newState, msgId) => {
      forceUpdate();
    });

    return () => {
      unsubscribe();
      serialRef.current?.disconnect();
      wsRef.current?.disconnect();
    };
  }, []);

  const [, forceUpdate] = React.useReducer(x => x + 1, 0);

  const handleData = (data) => {
    parserRef.current.feed(data);
    const packets = parserRef.current.parse();
    packets.forEach(packet => {
      stateManagerRef.current.update(packet);
    });
  };

  const handleError = (error) => {
    console.error('Connection error:', error);
    setConnected(false);
  };

  const handleConnect = async () => {
    if (connectionType === 'serial') {
      parserRef.current.setProtocol('firmware');
      const success = await serialRef.current.connect();
      setConnected(success);
    } else {
      parserRef.current.setProtocol('new');
      const success = wsRef.current.connect(wifiUrl);
      setConnected(success);
    }
  };

  const handleDisconnect = () => {
    if (connectionType === 'serial') {
      serialRef.current.disconnect();
    } else {
      wsRef.current.disconnect();
    }
    setConnected(false);
  };

  const handleSetPid = (axis, p, i, d) => {
    const packet = parserRef.current.encodeSetPid(axis, p, i, d);
    if (connectionType === 'serial') {
      serialRef.current.write(packet);
    } else {
      wsRef.current.write(packet);
    }
  };

  const handleGetPid = (axis) => {
    const packet = parserRef.current.encodeGetPid(axis);
    if (connectionType === 'serial') {
      serialRef.current.write(packet);
    } else {
      wsRef.current.write(packet);
    }
  };

  const handleSetMode = (mode) => {
    const packet = parserRef.current.encodeSetMode(mode);
    if (connectionType === 'serial') {
      serialRef.current.write(packet);
    } else {
      wsRef.current.write(packet);
    }
  };

  const handleArmDisarm = (arm) => {
    const packet = parserRef.current.encodeArmDisarm(arm);
    if (connectionType === 'serial') {
      serialRef.current.write(packet);
    } else {
      wsRef.current.write(packet);
    }
  };

  const isStale = stateManagerRef.current.isStale();

  const dotClass = connected ? 'dot dot-green' : isStale ? 'dot dot-red' : 'dot dot-yellow';
  const statusText = connected ? 'Connected' : isStale ? 'Stale' : 'Disconnected';

  return (
    <div className="app-container">
      <header className="header">
        <h1>OpenWing GCS</h1>
        <div className="header-controls">
          <select
            value={connectionType}
            onChange={(e) => setConnectionType(e.target.value)}
            className="select"
          >
            <option value="serial">USB Serial</option>
            <option value="wifi">WiFi</option>
          </select>

          {connectionType === 'wifi' && (
            <input
              type="text"
              value={wifiUrl}
              onChange={(e) => setWifiUrl(e.target.value)}
              placeholder="ws://192.168.4.1:8080"
              className="inp inp-sm"
              style={{ width: '180px' }}
            />
          )}

          <button
            onClick={handleConnect}
            disabled={connected}
            className="btn btn-success"
          >
            Connect
          </button>

          <button
            onClick={handleDisconnect}
            disabled={!connected}
            className="btn btn-danger"
          >
            Disconnect
          </button>

          <div className="connection-indicator card">
            <span className={dotClass} />
            <span>{statusText}</span>
          </div>
        </div>
      </header>

      <nav className="nav">
        {['dashboard', 'pid', 'compass', 'map'].map(tab => (
          <button
            key={tab}
            onClick={() => setActiveTab(tab)}
            className={`nav-tab${activeTab === tab ? ' active' : ''}`}
          >
            {tab === 'dashboard' ? 'Dashboard' : tab === 'pid' ? 'PID Tuning' : tab.charAt(0).toUpperCase() + tab.slice(1)}
          </button>
        ))}
      </nav>

      <main>
        {activeTab === 'dashboard' && <Dashboard state={state} />}
        {activeTab === 'pid' && <PIDPanel state={state} onSetPid={handleSetPid} onGetPid={handleGetPid} />}
        {activeTab === 'compass' && <CompassView state={state} />}
        {activeTab === 'map' && <MapView state={state} />}
      </main>

      <div className="footer">OpenWing Ground Control Station v1.0.0</div>
    </div>
  );
};

const container = document.getElementById('root');
const root = createRoot(container);
root.render(<App />);
