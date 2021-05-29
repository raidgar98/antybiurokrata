#include <antybiurokrata/libraries/generator/generator.h>
#include <xlsx/xlsxdocument.h>
#include <xlsx/xlsxworkbook.h>

namespace core
{
	namespace reports
	{
		generator::generator(source_data_t i_data, const str& i_filename, const empty_subscription_function_t i_on_finish, const subscription_function_t<size_t> i_on_progress) :
			 m_data{i_data}, m_filename{i_filename}
		{
			on_finish.register_slot(i_on_finish);
			on_progress.register_slot(i_on_progress);
		}


		void generator::process()
		{
			process_impl();
			on_progress(100);
			on_finish();
		}


		void generator::process_impl()
		{
			check_nullptr{ m_data };
			on_progress(0);

			// setting up sheet
			QXlsx::Document doc{};

			// setting up formating
			QXlsx::Format column_format;
			column_format.setShrinkToFit(true);

			QXlsx::Format default_cell{};
			default_cell.setBorderStyle(QXlsx::Format::BorderStyle::BorderThin);
			default_cell.setBorderIndex(1);

			QXlsx::Format aligned_cell{default_cell};
			aligned_cell.setHorizontalAlignment( QXlsx::Format::HorizontalAlignment::AlignHCenter );
			aligned_cell.setVerticalAlignment( QXlsx::Format::VerticalAlignment::AlignVCenter );

			QXlsx::Format header_format{ aligned_cell };
			header_format.setFontBold(true);

			QXlsx::Format full_match_format{ aligned_cell };
			full_match_format.setPatternBackgroundColor(QColor{ 78, 155, 71 });

			QXlsx::Format half_match_format{ aligned_cell };
			half_match_format.setPatternBackgroundColor(QColor{ 207, 173, 32 });

			QXlsx::Format no_match_format{ aligned_cell };
			no_match_format.setPatternBackgroundColor(QColor{ 183, 61, 68 });

			QXlsx::Format black_cell{ aligned_cell };
			black_cell.setPatternBackgroundColor(QColor{ 0, 0, 0 });


			// setting up A1
			const size_t first_row = 1, first_col = 1;
			size_t row = first_row, col = first_col;

			// quick access to enums
			// // id type
			using id_type_unit = objects::detail::id_type_translation_unit;
			const size_t id_length = id_type_unit::length;

			// // match type
			using match_type_unit = objects::detail::match_type_translation_unit;
			const size_t match_length = match_type_unit::length - 1;

			// setting up headers
			std::vector<QString> headers{ { QString{"Tytuł referencyjny"}, QString{"Tytuł alternatywny"}, QString{"Rok"} } };
			headers.reserve( match_length + id_length );

			std::map<uint8_t, size_t> ids_to_col;
			for(id_type_unit::base_enum_t i = 0; i < id_length; ++i)
			{
				headers.emplace_back( QString::fromStdU16String( id_type_unit::translation[i] ) );
				ids_to_col[i] = headers.size();
			}

			std::map<uint8_t, size_t> match_to_col;
			for(match_type_unit::base_enum_t i = 1; i < match_length; ++i)
			{
				headers.emplace_back( QString::fromStdU16String( match_type_unit::translation[i] ) );
				match_to_col[i] = headers.size();
			}

			// printing headers
			for(size_t i = 0; i < headers.size(); ++i)
			{
				doc.setColumnWidth( i + 1, 25 );
				dassert{ doc.write(first_row, i + 1, headers[i], header_format), "failed to insert header"_u8 };
			}
			doc.write(first_row, headers.size() + 1, "", black_cell);


			for(const auto& value : *m_data)
			{
				// begin loop
				++row;
				col = first_col;
				on_progress(1);


				// aliases
				const auto& ref = (*(*value())().reference()())();
				const auto& matched = (*value())().matched()().data();

				// reference title
				doc.write( row, col++, QString::fromStdU16String( ref.title()().raw ) );

				// alternative title
				if(!ref.polish_title()()->empty())
					doc.write( row, col, QString::fromStdU16String( ref.polish_title()().raw ) );
				col++;
				
				// year
				doc.write( row, col++, ref.year() );

				// ids
				for( const auto& id_pair : ref.ids()().data() )
					doc.write( row, ids_to_col[ static_cast<id_type_unit::base_enum_t>(id_pair.first) ], QString::fromStdU16String(id_pair.second().raw) );

				// matched
				for( const auto& match : matched )
				{
					doc.write( row, match_to_col[ static_cast<match_type_unit::base_enum_t>(match().source()().data())], true );
					for( const auto& id_pair : (*match().publication()().data())().ids()().data() )
						doc.write( row, ids_to_col[ static_cast<id_type_unit::base_enum_t>(id_pair.first) ], QString::fromStdU16String(id_pair.second().raw) );
				}

				if(matched.size() == 0) doc.setRowFormat(row, no_match_format);
				else if(matched.size() == match_length - 1) doc.setRowFormat(row, full_match_format);
				else doc.setRowFormat(row, half_match_format);

				doc.write(row, headers.size() + 1, "", black_cell);
			}

			dassert{ doc.saveAs(QString::fromStdString( m_filename )), "no access rights, failed to write"_u8 };
		}
	}	 // namespace reports
}	 // namespace core