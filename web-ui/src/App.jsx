import { useState, useEffect } from 'react'
import React from 'react';
import { ConfigProvider, Space, theme, Button, Flex, Layout, Alert } from 'antd';
const { Header, Footer, Sider, Content } = Layout;
import { WifiOutlined } from '@ant-design/icons';

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
  backgroundColor: 'rgb(0, 21, 41)',
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
    <ConfigProvider theme={{ algorithm: darkAlgorithm }}>
    <Flex gap="middle" wrap>
      <Layout style={layoutStyle}>
        <Header style={headerStyle}>{ device ? 'Settings' : 'My Machine' }</Header>
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
                style={{textAlign:'left'}}
              />
              : null }

            <Button onClick={()=>connect()} type="primary" icon={<WifiOutlined />} iconPosition='end' loading={loading}>Connect</Button>
          </Space> }
        </Content>
      </Layout>
    </Flex>
    </ConfigProvider>
  );
}

export default App;
