import { useState, useEffect } from 'react'
import React from 'react';
import { ConfigProvider, Space, theme, Button, Flex, Layout, Alert } from 'antd';
const { Header, Footer, Sider, Content } = Layout;

const BTIcon = () => (
  <svg xmlns="http://www.w3.org/2000/svg" style={{paddingTop:'2pt', marginRight:'-4pt'}} height="20pt" viewBox="0 -960 960 960" width="20pt" fill="#e8eaed"><path d="M440-80v-304L256-200l-56-56 224-224-224-224 56-56 184 184v-304h40l228 228-172 172 172 172L480-80h-40Zm80-496 76-76-76-74v150Zm0 342 76-74-76-76v150Z"/></svg>
);


import { Device } from './Device'

const OPENPAP_BLE_SERVICE = "ab119f42-66fa-11ef-99b5-2b0355ed36bb"

import './App.css'

const layoutStyle = {
  borderRadius: 8,
  overflow: 'hidden',
  width: 'calc(100% - 0px)',
  maxWidth: 'calc(100% - 0px)',
  minHeight: '100vh',
};

const headerStyle = {
  textAlign: 'center',
  color: '#ccc',
  height: 64,
  paddingInline: 48,
  lineHeight: '64px',
  backgroundColor: '#007d7d',
  fontSize: '200%',
  fontWeight: 'bold',
};

const contentStyle = {
  textAlign: 'center',
  minHeight: 120,
  backgroundColor: 'black',
};

const footerStyle = {
  textAlign: 'center',
  backgroundColor: '#111',
  color: '#ccc',
  padding: '.5em 1em'
};

const { darkAlgorithm } = theme;

const CHARACTERISTICS = [
  {
    name: 'version',
    display_name: 'Version',
    type: 'str',
    id: "7c816628-672a-11ef-a20f-535e28811a27",
  },
  {
    name: 'pressure',
    display_name: 'Pressure',
    type: 'float',
    id: "8387ecb2-551e-4665-83e4-7f0fffd1f850",
    units: 'cm H2O'
  },
  {
    name: 'kp',
    display_name: 'Kp',
    type: 'float',
    id: "998332a0-67c6-11ef-b2df-e78aa6930f15",
  },
  {
    name: 'ki',
    display_name: 'Ki',
    type: 'float',
    id: "9ee75a0a-67c6-11ef-adef-b367f4c7b993",
  },
  {
    name: 'kd',
    display_name: 'Kd',
    type: 'float',
    id: "9f4ad8b4-67c6-11ef-a327-57bd08d0ce3f",
  },
]


const App = () => {

  const [device, set_device] = useState(null)
  const [characteristics, set_characteristics] = useState(null)
  const [disconnected, set_disconnected] = useState(false)
  const [loading, set_loading] = useState(false)
 

  useEffect(()=>{
  }, []) 

  async function connect() {
    set_disconnected(false)
    set_loading(true)
    try {
      let device = await navigator.bluetooth.requestDevice({
          filters: [{ services: [OPENPAP_BLE_SERVICE] }]
      });
      const server = await device.gatt.connect();
      device.addEventListener('gattserverdisconnected', () => {
        set_device(null)
        set_disconnected(true)
        set_characteristics(null)
        set_loading(false)
      })
      const service = await server.getPrimaryService(OPENPAP_BLE_SERVICE);
      set_characteristics(await Promise.all(CHARACTERISTICS.map(async (c) => ({
        ...c,
        char: await service.getCharacteristic(c.id),
      }))))
      set_device(device)
      set_loading(false)
  } catch (error) {
      console.error('Connection failed!', error);
      set_loading(false)
    }
  }

  function disconnect() {
    if (confirm("Disconnect from your PAP machine?")) {
      if (device.gatt.connected) device.gatt.disconnect()
      set_device(null)
      }
  }

  return (
    <ConfigProvider theme={{
      "token": {
        "colorPrimary": "#007d7d",
        "colorInfo": "#007d7d"
      },
      "algorithm": darkAlgorithm
    }}>
    <Flex gap="middle" wrap>
      <Layout style={layoutStyle}>
        <Header style={headerStyle}>{ device ? 'Settings' : 'OpenPAP' }</Header>
        <Content style={contentStyle}>
          { device ? <>
              <Device characteristics={characteristics} />
              <Button type="text" style={{color:'#800'}} onClick={()=>disconnect()}>Disconnect</Button>
            </> : <Space direction="vertical">
            <img src='machine.png' style={{width:'100%', padding:'4em'}} />

            { disconnected ? 
              <Alert
                message="The Bluetooth connection to your PAP machine was disconnected."
                type="error"
                closable
                style={{textAlign:'left', margin:'1em'}}
              />
              : null }

            <Button onClick={()=>connect()} size='large' type="primary" icon={<BTIcon/>} iconPosition='end' loading={loading}>Connect</Button>
          </Space> }
        </Content>
      </Layout>
    </Flex>
    </ConfigProvider>
  );
}

export default App;
