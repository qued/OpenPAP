import { useState, useEffect } from 'react'
import { Alert, Button } from 'antd';
import { compare } from 'compare-versions';

export const Upgrader = (props) => {

  const [latest_version, set_latest_version] = useState(null)

    useEffect(()=>{

      async function get_releases() {
        try {
          const response = await fetch("https://openpap.org/arduino/VERSION");
          if (!response.ok) {
            throw new Error(`Response status: ${response.status}`);
          }
          const latest_version = (await response.text()).trim();
          console.log('latest_version', latest_version);
          set_latest_version(latest_version)
        } catch (error) {
          console.error(error.message);
        }
      }

      get_releases()
          
    }, [])

    if (props.version && compare(latest_version, props.version, '='))
    return (
      <Alert
        style={{margin:'1em', textAlign:'left'}}
        message={"Upgrade to v"+ latest_version +" Available!"}
        type="success"
        showIcon
        action={
          <Button size="small" type="">
            Upgrade
          </Button>
        }
        closable
      />
      )
}