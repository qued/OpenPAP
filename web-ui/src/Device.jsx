import { useState, useEffect } from 'react'
import { List, Spin, Flex, InputNumber } from 'antd';
import { EditOutlined, StopOutlined, CloseOutlined, SaveOutlined } from '@ant-design/icons';



export const Device = (props) => {

  const [values, set_values] = useState({})

  useEffect(()=>{
    let new_values = {}
    Promise.all(props.characteristics.map(async(characteristic) => {
      let bytes = await characteristic.char.readValue();
      var v = null;
      let decoder = new TextDecoder('utf-8');
      v = decoder.decode(bytes);
      if (characteristic.type=='float') {
        v = parseFloat(v)
      }
      new_values[characteristic.id] = v
    })).then(()=> set_values(new_values))
  }, [props.characteristics]) 

  return (
    <List style={{textAlign:'left'}}
      size='large'
      dataSource={props.characteristics}
      renderItem={(characteristic) => <Characteristic {...characteristic} value={values[characteristic.id]} />}
    />
  );
}

const Characteristic = (props) => {
  const [value, set_value] = useState(null)
  useEffect(()=>{
    set_value(props.value)
  }, [props.value]) 
  const [editing, set_editing] = useState(false)
  const [input_value, set_input_value] = useState(null)
  const [saving, set_saving] = useState(false)
  let save = async () => {
    console.log('save', input_value)
    set_saving(true)
    await props.char.writeValue(new TextEncoder().encode(input_value));
    set_value(input_value)
    set_editing(false)
    set_saving(false)
  }
  let actions = []
  if (saving || !props.value) actions.push(<Spin />)
  else {
    if (editing) {
      actions.push(<SaveOutlined style={{color:'green'}} onClick={()=>save()}/>)
      actions.push(<CloseOutlined style={{color:'red'}} onClick={()=>set_editing(false)}/>)
    } else if (props.char.properties.write) {
      actions.push(<EditOutlined onClick={()=>set_editing(true) || set_input_value(value)}/>)
    }
  }
  return (
    <List.Item actions={actions}>
      <Flex justify='space-between' style={{flexGrow:1}}>
        <div>{props.name}:</div>
        { editing ? <InputNumber disabled={saving} autoFocus precision={2} min={1} max={20} onPressEnter={()=>save()} value={input_value} onChange={(v)=>set_input_value(v)} /> : <div>{value} {props.units}</div> }
      </Flex>
    </List.Item>
  )
}