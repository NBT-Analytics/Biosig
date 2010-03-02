/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "FEF-IntermediateDraft"
 * 	found in "../annexb-snacc-122001.asn1"
 */

#ifndef	_DescriptiveDataSection_H_
#define	_DescriptiveDataSection_H_


#include <asn_application.h>

/* Including external dependencies */
#include "Handle.h"
#include "Placeholder.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct RealTimeSampleArrayDescriptiveDataSection;
struct TimeSampleArrayDescriptiveDataSection;
struct DistributionSampleArrayDescriptiveDataSection;
struct NumericDescriptiveDataSection;
struct EnumerationDescriptiveDataSection;

/* DescriptiveDataSection */
typedef struct DescriptiveDataSection {
	Handle_t	 handle;
	Placeholder_t	*placeholder	/* OPTIONAL */;
	struct DescriptiveDataSection__realtimesadescs {
		A_SEQUENCE_OF(struct RealTimeSampleArrayDescriptiveDataSection) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *realtimesadescs;
	struct DescriptiveDataSection__timesadescs {
		A_SEQUENCE_OF(struct TimeSampleArrayDescriptiveDataSection) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *timesadescs;
	struct DescriptiveDataSection__distributionsadescs {
		A_SEQUENCE_OF(struct DistributionSampleArrayDescriptiveDataSection) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *distributionsadescs;
	struct DescriptiveDataSection__numericdescs {
		A_SEQUENCE_OF(struct NumericDescriptiveDataSection) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *numericdescs;
	struct DescriptiveDataSection__enumerationdescs {
		A_SEQUENCE_OF(struct EnumerationDescriptiveDataSection) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *enumerationdescs;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DescriptiveDataSection_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DescriptiveDataSection;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "RealTimeSampleArrayDescriptiveDataSection.h"
#include "TimeSampleArrayDescriptiveDataSection.h"
#include "DistributionSampleArrayDescriptiveDataSection.h"
#include "NumericDescriptiveDataSection.h"
#include "EnumerationDescriptiveDataSection.h"

#endif	/* _DescriptiveDataSection_H_ */