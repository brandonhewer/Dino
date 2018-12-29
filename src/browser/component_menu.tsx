import React from 'react';

interface DraggableIconProps {
  iconClass : string;
  height : number;
  width : number;
}

function DraggableIcon(props : DraggableIconProps) : JSX.Element {
  
}

interface ComponentIconProps {
  iconClass : string;
  label : string;
  height : number;
  width : number;
}

function ComponentIcon(props : ComponentIconProps) : JSX.Element {
  return (
    <button className='component-icon'>
      <span className={props.iconClass} role='img' aria-label={props.label} 
            style={{'width' : props.width, 'height' : props.height}}>
      </span>
    </button>
  );
}

interface ComponentRowProps {
  icons : JSX.Element[];
}

function ComponentRow(props : ComponentRowProps) : JSX.Element {
  return (
    <div id='component-row'>
      {props.icons}
    </div>
  );
}

interface ComponentSelectionProps {
  icons : JSX.Element[];
  iconsInRow : number;
};

function range(N : number) : number[] {
  return Array.from(Array(N).keys());
}

function subslices<T>(list : T[], cutEvery : number) : T[][] {
  const getSlice = (i : number) => list.slice(i * cutEvery, (i + 1) * cutEvery);
  return range(Math.ceil(list.length / cutEvery)).map(getSlice);
}

function ComponentSelection(props : ComponentSelectionProps) : JSX.Element {
  return (
    <div id='component-selection'>
      {subslices(props.icons, props.iconsInRow)}
    </div>
  );
}


