import { useContext } from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row, Col } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { ANALOG_PINS } from '../Data/Buttons';
import AnalogPinOptions from '../Components/AnalogPinOptions';
import { AppContext } from '../Contexts/AppContext';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

// Button mask values matching C++ GAMEPAD_MASK defines
const BUTTON_MASKS = [
	{ label: 'B1 (A / Cross)', value: 0x01 },
	{ label: 'B2 (B / Circle)', value: 0x02 },
	{ label: 'B3 (X / Square)', value: 0x04 },
	{ label: 'B4 (Y / Triangle)', value: 0x08 },
	{ label: 'L1 (LB)', value: 0x10 },
	{ label: 'R1 (RB)', value: 0x20 },
	{ label: 'L2 (LT)', value: 0x40 },
	{ label: 'R2 (RT)', value: 0x80 },
	{ label: 'S1 (Select / Share)', value: 0x100 },
	{ label: 'S2 (Start / Options)', value: 0x200 },
	{ label: 'L3 (LS)', value: 0x400 },
	{ label: 'R3 (RS)', value: 0x800 },
	{ label: 'A1 (Home / PS)', value: 0x1000 },
	{ label: 'A2 (Capture)', value: 0x2000 },
];

export const taikoScheme = {
	TaikoEnabled: yup.number().required().label('Taiko Enabled'),

	// Sensor pin assignments
	sensor1Pin: yup
		.number()
		.label('Sensor 1 Pin')
		.validatePinWhenValue('TaikoEnabled'),
	sensor2Pin: yup
		.number()
		.label('Sensor 2 Pin')
		.validatePinWhenValue('TaikoEnabled'),
	sensor3Pin: yup
		.number()
		.label('Sensor 3 Pin')
		.validatePinWhenValue('TaikoEnabled'),
	sensor4Pin: yup
		.number()
		.label('Sensor 4 Pin')
		.validatePinWhenValue('TaikoEnabled'),

	// Button mappings
	sensor1Button: yup
		.number()
		.label('Sensor 1 Button')
		.validateRangeWhenValue('TaikoEnabled', 0, 0xffff),
	sensor2Button: yup
		.number()
		.label('Sensor 2 Button')
		.validateRangeWhenValue('TaikoEnabled', 0, 0xffff),
	sensor3Button: yup
		.number()
		.label('Sensor 3 Button')
		.validateRangeWhenValue('TaikoEnabled', 0, 0xffff),
	sensor4Button: yup
		.number()
		.label('Sensor 4 Button')
		.validateRangeWhenValue('TaikoEnabled', 0, 0xffff),

	// Light thresholds
	sensor1ThresholdLight: yup
		.number()
		.label('Sensor 1 Light Threshold')
		.validateRangeWhenValue('TaikoEnabled', 0, 4095),
	sensor2ThresholdLight: yup
		.number()
		.label('Sensor 2 Light Threshold')
		.validateRangeWhenValue('TaikoEnabled', 0, 4095),
	sensor3ThresholdLight: yup
		.number()
		.label('Sensor 3 Light Threshold')
		.validateRangeWhenValue('TaikoEnabled', 0, 4095),
	sensor4ThresholdLight: yup
		.number()
		.label('Sensor 4 Light Threshold')
		.validateRangeWhenValue('TaikoEnabled', 0, 4095),

	// Heavy thresholds
	sensor1ThresholdHeavy: yup
		.number()
		.label('Sensor 1 Heavy Threshold')
		.validateRangeWhenValue('TaikoEnabled', 0, 4095),
	sensor2ThresholdHeavy: yup
		.number()
		.label('Sensor 2 Heavy Threshold')
		.validateRangeWhenValue('TaikoEnabled', 0, 4095),
	sensor3ThresholdHeavy: yup
		.number()
		.label('Sensor 3 Heavy Threshold')
		.validateRangeWhenValue('TaikoEnabled', 0, 4095),
	sensor4ThresholdHeavy: yup
		.number()
		.label('Sensor 4 Heavy Threshold')
		.validateRangeWhenValue('TaikoEnabled', 0, 4095),

	// Timing configuration
	debounceMillis: yup
		.number()
		.label('Debounce (ms)')
		.validateRangeWhenValue('TaikoEnabled', 1, 1000),
	keyTimeoutMillis: yup
		.number()
		.label('Key Timeout (ms)')
		.validateRangeWhenValue('TaikoEnabled', 1, 1000),

	// Anti-ghosting
	antiGhostingSides: yup
		.number()
		.label('Anti-Ghosting Sides')
		.validateRangeWhenValue('TaikoEnabled', 0, 1),
	antiGhostingCenter: yup
		.number()
		.label('Anti-Ghosting Center')
		.validateRangeWhenValue('TaikoEnabled', 0, 1),
};

export const taikoState = {
	TaikoEnabled: 0,

	// Default pins (disabled)
	sensor1Pin: -1,
	sensor2Pin: -1,
	sensor3Pin: -1,
	sensor4Pin: -1,

	// Default button mappings (B1-B4 = A/B/X/Y)
	sensor1Button: 0x01,
	sensor2Button: 0x02,
	sensor3Button: 0x04,
	sensor4Button: 0x08,

	// Default light thresholds (matching STM32 code)
	sensor1ThresholdLight: 1400,
	sensor2ThresholdLight: 600,
	sensor3ThresholdLight: 700,
	sensor4ThresholdLight: 1400,

	// Default heavy thresholds (matching STM32 code)
	sensor1ThresholdHeavy: 3600,
	sensor2ThresholdHeavy: 2600,
	sensor3ThresholdHeavy: 2700,
	sensor4ThresholdHeavy: 3600,

	// Default timing (matching STM32 code)
	debounceMillis: 45,
	keyTimeoutMillis: 30,

	// Default anti-ghosting (enabled)
	antiGhostingSides: 1,
	antiGhostingCenter: 1,
};

// SensorConfig component moved outside to prevent re-creation on each render
const SensorConfig = ({
	sensorNum,
	sensorName,
	values,
	errors,
	handleChange,
}: {
	sensorNum: number;
	sensorName: string;
	values: any;
	errors: any;
	handleChange: any;
}) => {
	const pinField = `sensor${sensorNum}Pin`;
	const buttonField = `sensor${sensorNum}Button`;
	const lightField = `sensor${sensorNum}ThresholdLight`;
	const heavyField = `sensor${sensorNum}ThresholdHeavy`;

	return (
		<div className="mb-4 p-3 border rounded">
			<h5 className="mb-3">{sensorName}</h5>
			<Row className="mb-3">
				<FormSelect
					label="ADC Pin"
					name={pinField}
					className="form-select-sm"
					groupClassName="col-sm-4"
					value={values[pinField]}
					error={errors[pinField]}
					isInvalid={Boolean(errors[pinField])}
					onChange={handleChange}
				>
					<AnalogPinOptions />
				</FormSelect>
				<FormSelect
					label="Button Mapping"
					name={buttonField}
					className="form-select-sm"
					groupClassName="col-sm-8"
					value={values[buttonField]}
					error={errors[buttonField]}
					isInvalid={Boolean(errors[buttonField])}
					onChange={handleChange}
				>
					{BUTTON_MASKS.map((option, i) => (
						<option key={`${buttonField}-option-${i}`} value={option.value}>
							{option.label}
						</option>
					))}
				</FormSelect>
			</Row>
			<Row>
				<FormControl
					type="number"
					label="Light Hit Threshold (0-4095)"
					name={lightField}
					className="form-control-sm"
					groupClassName="col-sm-6"
					value={values[lightField]}
					error={errors[lightField]}
					isInvalid={Boolean(errors[lightField])}
					onChange={handleChange}
					min={0}
					max={4095}
				/>
				<FormControl
					type="number"
					label="Heavy Hit Threshold (0-4095)"
					name={heavyField}
					className="form-control-sm"
					groupClassName="col-sm-6"
					value={values[heavyField]}
					error={errors[heavyField]}
					isInvalid={Boolean(errors[heavyField])}
					onChange={handleChange}
					min={0}
					max={4095}
				/>
			</Row>
		</div>
	);
};

const Taiko = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
}: AddonPropTypes) => {
	const { usedPins } = useContext(AppContext);
	const { t } = useTranslation();
	const availableAnalogPins = ANALOG_PINS.filter(
		(pin) => !usedPins?.includes(pin),
	);

	return (
		<Section title="Taiko Drum Controller">
			<div id="TaikoOptions" hidden={!values.TaikoEnabled}>
				<div className="alert alert-info" role="alert">
					<strong>Taiko Mode:</strong> Connect piezo drum sensors to ADC pins.
					Features dual-threshold detection, anti-ghosting, and configurable
					debouncing.
				</div>
				<div className="alert alert-success" role="alert">
					Available ADC pins: {availableAnalogPins.join(', ')}
				</div>

				<h5 className="mt-4 mb-3">Sensor Configuration</h5>
				<SensorConfig sensorNum={1} sensorName="Sensor 1 (Left Side)" values={values} errors={errors} handleChange={handleChange} />
				<SensorConfig sensorNum={2} sensorName="Sensor 2 (Center Left)" values={values} errors={errors} handleChange={handleChange} />
				<SensorConfig sensorNum={3} sensorName="Sensor 3 (Center Right)" values={values} errors={errors} handleChange={handleChange} />
				<SensorConfig sensorNum={4} sensorName="Sensor 4 (Right Side)" values={values} errors={errors} handleChange={handleChange} />

				<h5 className="mt-4 mb-3">Timing Configuration</h5>
				<Row className="mb-3">
					<FormControl
						type="number"
						label="Debounce Time (ms)"
						name="debounceMillis"
						className="form-control-sm"
						groupClassName="col-sm-6"
						value={values.debounceMillis}
						error={errors.debounceMillis}
						isInvalid={Boolean(errors.debounceMillis)}
						onChange={handleChange}
						min={1}
						max={1000}
					/>
					<FormControl
						type="number"
						label="Key Release Timeout (ms)"
						name="keyTimeoutMillis"
						className="form-control-sm"
						groupClassName="col-sm-6"
						value={values.keyTimeoutMillis}
						error={errors.keyTimeoutMillis}
						isInvalid={Boolean(errors.keyTimeoutMillis)}
						onChange={handleChange}
						min={1}
						max={1000}
					/>
				</Row>
				<div className="alert alert-secondary" role="alert">
					<small>
						<strong>Debounce:</strong> Minimum time between normal hits (ms).
						Heavy hits bypass this delay.
						<br />
						<strong>Timeout:</strong> How long to hold the button press after
						the hit is detected (ms).
					</small>
				</div>

				<h5 className="mt-4 mb-3">Anti-Ghosting</h5>
				<Row className="mb-3">
					<Col sm={6}>
						<FormCheck
							label="Prevent Side Sensors (1/4) when Center Active"
							type="switch"
							id="antiGhostingSides"
							isInvalid={false}
							checked={Boolean(values.antiGhostingSides)}
							onChange={() => {
								handleCheckbox('antiGhostingSides');
							}}
						/>
					</Col>
					<Col sm={6}>
						<FormCheck
							label="Prevent Center Sensors (2/3) when Sides Active"
							type="switch"
							id="antiGhostingCenter"
							isInvalid={false}
							checked={Boolean(values.antiGhostingCenter)}
							onChange={() => {
								handleCheckbox('antiGhostingCenter');
							}}
						/>
					</Col>
				</Row>
				<div className="alert alert-secondary" role="alert">
					<small>
						Anti-ghosting prevents adjacent sensors from triggering
						simultaneously, reducing false inputs from drum vibrations.
					</small>
				</div>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="TaikoButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.TaikoEnabled)}
				onChange={(e) => {
					handleCheckbox('TaikoEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Taiko;
