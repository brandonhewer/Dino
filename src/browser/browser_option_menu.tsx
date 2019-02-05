import { MouseEvent } from 'react';

interface IBrowserOptionSetup {
  readonly iconClass: string;
  readonly label: string;
  readonly height: number;
  readonly width: number;
}

interface IBrowserOptionProps extends IBrowserOptionSetup {
  readonly onClick: (e: MouseEvent) => any;
}

function BrowserOption(props: IBrowserOptionProps): JSX.Element {
  return (
    <button className="browser-option" onClick={props.onClick}>
      <span
        className={props.iconClass}
        role="img"
        aria-label={props.label}
        style={{ width : props.width, height : props.height }}
      />
    </button>
  );
}

export interface IBrowserOptionMenuProps {
  readonly iconClasses: string[];
  readonly labels: string[];
  readonly optionWidth: number;
  readonly optionHeight: number;
  readonly onClick: (i: number) => (e: MouseEvent) => any;
}

function createBrowserOptions(
  iconClasses: string[],
  labels: string[],
  width: number,
  height: number,
  onClick: (i: number) => (e: MouseEvent) => any): JSX.Element[] {
  return iconClasses.map(
    (iconClass, index) => (
      <BrowserOption
        iconClass={iconClass}
        height={height}
        width={width}
        label={labels[index]}
        onClick={onClick(index)}
      />
    ),
  );
}

export function BrowserOptionMenu(props: IBrowserOptionMenuProps): JSX.Element {
  const options = createBrowserOptions(props.iconClasses, props.labels,
                                       props.optionWidth, props.optionHeight,
                                       props.onClick);
  return (
    <div id="browser-option-menu">
      {options}
    </div>
  );
}
