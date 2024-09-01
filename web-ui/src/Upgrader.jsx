import { useState, useEffect } from 'react'
import { Alert, Progress, Button, Modal, Input } from 'antd';
import { compare } from 'compare-versions';

function delay(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

export const Upgrader = (props) => {

  const [latest_version, set_latest_version] = useState(null)
  const [modal_open, set_modal_open] = useState(false)
  const [progress, set_progress] = useState(null)
  const [progress_exception, set_progress_exception] = useState(null)
  const [ssid, set_ssid] = useState(null)
  const [pass, set_pass] = useState(null)

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

      return () => {
        if (progress) console.log('upgrade complete')
      }
          
    }, [])

    async function do_upgrade() {
      var progress = 0
      set_progress(progress)
      props.upgrade(ssid, pass)
      while(modal_open) {
        let bytes = await props.char.char.readValue();
        var v = null;
        let decoder = new TextDecoder('utf-8');
        v = decoder.decode(bytes);
        let i = parseInt(v, 10);
        set_progress(i)
        set_progress_exception(i==NaN ? 'exception' : null)
        console.log('progress', progress)
        await delay(1000)
      }
    }

    if (props.version && compare(latest_version, props.version, '>'))
    return (
      <>
        <Modal title="Upgrade Your CPAP Machine" open={modal_open} onOk={()=>do_upgrade()} onCancel={()=>set_modal_open(false)} okText='Upgrade' okButtonProps={{disabled: progress>0,}} cancelButtonProps={{disabled: progress>0,}}>
          <p>This will update your machine from <b>OpenPAP v{props.version}</b> to <b>v{latest_version}</b>.</p>
          {progress==null ? <>
            <div><Input autoComplete={false} name='ssid' placeholder='SSID...' value={ssid} onChange={e=>set_ssid(e.target.value)}/></div>
            <p><Input.Password autoComplete={false} name='ssid-password' placeholder='Password...' value={pass} onChange={e=>set_pass(e.target.value)}/></p>
          </> : <div style={{textAlign:'center'}}>
            <Progress type="circle" percent={progress} exception={progress_exception} />
          </div>}
          <p>After the upgrade your machine will reboot.</p>
        </Modal>

        <Alert
        style={{margin:'1em', textAlign:'left'}}
        message={"Upgrade to v"+ latest_version +" Available!"}
        type="success"
        showIcon
        action={
          <Button size="small" type="" onClick={()=>set_modal_open(true)}>
            Upgrade
          </Button>
        }
        closable
      />
      </>
      )
}
